// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <button.h>
#include <clk.h>
#include <dm.h>
#include <dm/of_extra.h>
#include <env.h>
#include <env_internal.h>
#include <event.h>
#include <fdt_support.h>
#include <init.h>
#include <led.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/string.h>
#include <miiphy.h>
#include <spi.h>
#include <spi_flash.h>

#include "mox_sp.h"

#define MAX_MOX_MODULES		10

#define MOX_MODULE_SFP		0x1
#define MOX_MODULE_PCI		0x2
#define MOX_MODULE_TOPAZ	0x3
#define MOX_MODULE_PERIDOT	0x4
#define MOX_MODULE_USB3		0x5
#define MOX_MODULE_PASSPCI	0x6

#define ARMADA_37XX_NB_GPIO_SEL	(MVEBU_REGISTER(0x13830))
#define ARMADA_37XX_SPI_CTRL	(MVEBU_REGISTER(0x10600))
#define ARMADA_37XX_SPI_CFG	(MVEBU_REGISTER(0x10604))
#define ARMADA_37XX_SPI_DOUT	(MVEBU_REGISTER(0x10608))
#define ARMADA_37XX_SPI_DIN	(MVEBU_REGISTER(0x1060c))

DECLARE_GLOBAL_DATA_PTR;

int board_fit_config_name_match(const char *name)
{
	if (!gd->board_type) {
		enum cznic_a3720_board board;

		if (mbox_sp_get_board_info(NULL, NULL, NULL, NULL, NULL,
					   &board) < 0) {
			printf("Cannot determine board, defaulting to Turris MOX!\n");
			board = BOARD_TURRIS_MOX;
		}

		gd->board_type = board;
	}

	return !((gd->board_type == BOARD_TURRIS_MOX &&
		  !strcmp(name, "armada-3720-turris-mox")) ||
		 (gd->board_type == BOARD_RIPE_ATLAS &&
		  !strcmp(name, "armada-3720-ripe-atlas")));
}

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	enum fdt_status status_pcie, status_eth1;
	u8 topology[MAX_MOX_MODULES];
	int i, size, ret;
	bool eth1_sgmii;

	if (gd->board_type != BOARD_TURRIS_MOX)
		return 0;

	/*
	 * SPI driver is not loaded in driver model yet, but we have to find out
	 * if pcie should be enabled in U-Boot's device tree. Therefore we have
	 * to read SPI by reading/writing SPI registers directly
	 */

	/* put pin from GPIO to SPI mode */
	clrbits_le32(ARMADA_37XX_NB_GPIO_SEL, BIT(12));
	/* configure cpol, cpha, prescale */
	writel(0x10df, ARMADA_37XX_SPI_CFG);
	mdelay(1);
	/* enable SPI CS1 */
	setbits_le32(ARMADA_37XX_SPI_CTRL, BIT(17));

	while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
		udelay(1);

	status_pcie = FDT_STATUS_DISABLED;
	status_eth1 = FDT_STATUS_DISABLED;
	eth1_sgmii = false;

	for (i = 0; i < MAX_MOX_MODULES; ++i) {
		writel(0x0, ARMADA_37XX_SPI_DOUT);

		while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
			udelay(1);

		topology[i] = readl(ARMADA_37XX_SPI_DIN) & 0xff;
		if (topology[i] == 0xff)
			break;

		topology[i] &= 0xf;

		if (topology[i] == MOX_MODULE_SFP &&
		    status_pcie == FDT_STATUS_DISABLED)
			eth1_sgmii = true;

		if (topology[i] == MOX_MODULE_SFP ||
		    topology[i] == MOX_MODULE_TOPAZ ||
		    topology[i] == MOX_MODULE_PERIDOT)
			status_eth1 = FDT_STATUS_OKAY;
	}

	size = i;

	/* disable SPI CS1 */
	clrbits_le32(ARMADA_37XX_SPI_CTRL, BIT(17));

	ret = fdt_set_status_by_alias(blob, "ethernet1", status_eth1);
	if (ret < 0)
		printf("Cannot set status for eth1 in U-Boot's device tree: %s!\n",
		       fdt_strerror(ret));

	if (eth1_sgmii) {
		ret = fdt_path_offset(blob, "ethernet1");
		if (ret >= 0)
			ret = fdt_setprop_string(blob, ret, "phy-mode", "sgmii");
		if (ret < 0)
			printf("Cannot set phy-mode for eth1 to sgmii in U-Boot device tree: %s!\n",
			       fdt_strerror(ret));
	}

	if (size > 1 && (topology[1] == MOX_MODULE_PCI ||
			 topology[1] == MOX_MODULE_USB3 ||
			 topology[1] == MOX_MODULE_PASSPCI))
		status_pcie = FDT_STATUS_OKAY;

	ret = fdt_set_status_by_compatible(blob, "marvell,armada-3700-pcie",
					   status_pcie);
	if (ret < 0) {
		printf("Cannot set status for PCIe in U-Boot's device tree: %s!\n",
		       fdt_strerror(ret));
		return 0;
	}

	if (a3700_fdt_fix_pcie_regions(blob) < 0) {
		printf("Cannot fix PCIe regions in U-Boot's device tree!\n");
		return 0;
	}

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

static int mox_do_spi(u8 *in, u8 *out, size_t size)
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = _spi_get_bus_and_cs(0, 1, 1000000, SPI_CPHA | SPI_CPOL,
				  "spi_generic_drv", "moxtet@1", &dev,
				  &slave);
	if (ret)
		goto fail;

	ret = spi_claim_bus(slave);
	if (ret)
		goto fail_free;

	ret = spi_xfer(slave, size * 8, out, in, SPI_XFER_ONCE);

	spi_release_bus(slave);
fail_free:
	spi_free_slave(slave);
fail:
	return ret;
}

static int mox_get_topology(const u8 **ptopology, int *psize, int *pis_sd)
{
	static int is_sd;
	static u8 topology[MAX_MOX_MODULES - 1];
	static int size;
	u8 din[MAX_MOX_MODULES], dout[MAX_MOX_MODULES];
	int ret, i;

	if (size) {
		if (ptopology)
			*ptopology = topology;
		if (psize)
			*psize = size;
		if (pis_sd)
			*pis_sd = is_sd;
		return 0;
	}

	memset(din, 0, MAX_MOX_MODULES);
	memset(dout, 0, MAX_MOX_MODULES);

	ret = mox_do_spi(din, dout, MAX_MOX_MODULES);
	if (ret)
		return ret;

	if (din[0] == 0x10)
		is_sd = 1;
	else if (din[0] == 0x00)
		is_sd = 0;
	else
		return -ENODEV;

	for (i = 1; i < MAX_MOX_MODULES && din[i] != 0xff; ++i)
		topology[i - 1] = din[i] & 0xf;
	size = i - 1;

	if (ptopology)
		*ptopology = topology;
	if (psize)
		*psize = size;
	if (pis_sd)
		*pis_sd = is_sd;

	return 0;
}

#define SW_SMI_CMD_R(d, r)	(0x9800 | (((d) & 0x1f) << 5) | ((r) & 0x1f))
#define SW_SMI_CMD_W(d, r)	(0x9400 | (((d) & 0x1f) << 5) | ((r) & 0x1f))

static int sw_multi_read(struct udevice *bus, int sw, int dev, int reg)
{
	dm_mdio_write(bus, sw, MDIO_DEVAD_NONE, 0, SW_SMI_CMD_R(dev, reg));
	mdelay(5);
	return dm_mdio_read(bus, sw, MDIO_DEVAD_NONE, 1);
}

static void sw_multi_write(struct udevice *bus, int sw, int dev, int reg,
			   u16 val)
{
	dm_mdio_write(bus, sw, MDIO_DEVAD_NONE, 1, val);
	dm_mdio_write(bus, sw, MDIO_DEVAD_NONE, 0, SW_SMI_CMD_W(dev, reg));
	mdelay(5);
}

static int sw_scratch_read(struct udevice *bus, int sw, int reg)
{
	sw_multi_write(bus, sw, 0x1c, 0x1a, (reg & 0x7f) << 8);
	return sw_multi_read(bus, sw, 0x1c, 0x1a) & 0xff;
}

static void sw_led_write(struct udevice *bus, int sw, int port, int reg,
			 u16 val)
{
	sw_multi_write(bus, sw, port, 0x16, 0x8000 | ((reg & 7) << 12)
					    | (val & 0x7ff));
}

static void sw_blink_leds(struct udevice *bus, int peridot, int topaz)
{
	int i, p;
	struct {
		int port;
		u16 val;
		int wait;
	} regs[] = {
		{ 2, 0xef, 1 }, { 2, 0xfe, 1 }, { 2, 0x33, 0 },
		{ 4, 0xef, 1 }, { 4, 0xfe, 1 }, { 4, 0x33, 0 },
		{ 3, 0xfe, 1 }, { 3, 0xef, 1 }, { 3, 0x33, 0 },
		{ 1, 0xfe, 1 }, { 1, 0xef, 1 }, { 1, 0x33, 0 }
	};

	for (i = 0; i < 12; ++i) {
		for (p = 0; p < peridot; ++p) {
			sw_led_write(bus, 0x10 + p, regs[i].port, 0,
				     regs[i].val);
			sw_led_write(bus, 0x10 + p, regs[i].port + 4, 0,
				     regs[i].val);
		}
		if (topaz) {
			sw_led_write(bus, 0x2, 0x10 + regs[i].port, 0,
				     regs[i].val);
		}

		if (regs[i].wait)
			mdelay(75);
	}
}

static void check_switch_address(struct udevice *bus, int addr)
{
	if (sw_scratch_read(bus, addr, 0x70) >> 3 != addr)
		printf("Check of switch MDIO address failed for 0x%02x\n",
		       addr);
}

static int sfp, pci, topaz, peridot, usb, passpci;
static int sfp_pos, peridot_pos[3];
static int module_count;

static int configure_peridots(struct gpio_desc *reset_gpio)
{
	int i, ret;
	u8 dout[MAX_MOX_MODULES];

	memset(dout, 0, MAX_MOX_MODULES);

	/* set addresses of Peridot modules */
	for (i = 0; i < peridot; ++i)
		dout[module_count - peridot_pos[i]] = (~i) & 3;

	/*
	 * if there is a SFP module connected to the last Peridot module, set
	 * the P10_SMODE to 1 for the Peridot module
	 */
	if (sfp)
		dout[module_count - peridot_pos[i - 1]] |= 1 << 3;

	dm_gpio_set_value(reset_gpio, 1);
	mdelay(10);

	ret = mox_do_spi(NULL, dout, module_count + 1);

	mdelay(10);
	dm_gpio_set_value(reset_gpio, 0);

	mdelay(50);

	return ret;
}

static int get_reset_gpio(struct gpio_desc *reset_gpio)
{
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "cznic,moxtet");
	if (node < 0) {
		printf("Cannot find Moxtet bus device node!\n");
		return -1;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(node), "reset-gpios", 0,
				   reset_gpio, GPIOD_IS_OUT);

	if (!dm_gpio_is_valid(reset_gpio)) {
		printf("Cannot find reset GPIO for Moxtet bus!\n");
		return -1;
	}

	return 0;
}

/* Load default system DTB binary to $fdr_addr */
static void load_spi_dtb(void)
{
	const char *const env_name[1] = { "fdt_addr" };
	unsigned long size, offset;
	struct udevice *spi_dev;
	struct spi_flash *flash;
	const char *addr_str;
	unsigned long addr;
	void *buf;

	addr_str = env_get(env_name[0]);
	if (!addr_str) {
		env_set_default_vars(1, (char * const *)env_name, 0);
		addr_str = env_get(env_name[0]);
	}

	if (!addr_str)
		return;

	addr = hextoul(addr_str, NULL);
	if (!addr)
		return;

	spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, &spi_dev);
	flash = dev_get_uclass_priv(spi_dev);
	if (!flash)
		return;

	/*
	 * SPI NOR "dtb" partition offset & size hardcoded for now because the
	 * mtd subsystem does not offer finding the partition yet and we do not
	 * want to reimplement OF partition parser here.
	 */
	offset = 0x7f0000;
	size = 0x10000;

	buf = map_physmem(addr, size, MAP_WRBACK);
	if (!buf)
		return;

	spi_flash_read(flash, offset, size, buf);
	unmap_physmem(buf, size);
}

int misc_init_r(void)
{
	int i, ret, addrcnt;
	u8 mac[2][6];

	ret = mbox_sp_get_board_info(NULL, mac[0], mac[1], NULL, NULL, NULL);
	if (ret < 0) {
		printf("Cannot read data from OTP!\n");
		return 0;
	}

	if (gd->board_type == BOARD_TURRIS_MOX)
		addrcnt = 2;
	else if (gd->board_type == BOARD_RIPE_ATLAS)
		addrcnt = 1;
	else
		addrcnt = 0;

	for (i = 0; i < addrcnt; ++i) {
		u8 oldmac[6];

		if (is_valid_ethaddr(mac[i]) &&
		    !eth_env_get_enetaddr_by_index("eth", i, oldmac))
			eth_env_set_enetaddr_by_index("eth", i, mac[i]);
	}

	if (gd->board_type == BOARD_RIPE_ATLAS) {
		env_set("board", "ripe_atlas");
		env_set("board_name", "ripe_atlas");
		env_set("fdtfile", "marvell/armada-3720-ripe-atlas.dtb");
	} else {
		load_spi_dtb();
	}

	return 0;
}

static void mox_phy_modify(struct phy_device *phydev, int page, int reg,
			   u16 mask, u16 set)
{
	int val;

	val = phydev->drv->readext(phydev, MDIO_DEVAD_NONE, page, reg);
	val &= ~mask;
	val |= set;
	phydev->drv->writeext(phydev, MDIO_DEVAD_NONE, page, reg, val);
}

static void mox_phy_leds_start_blinking(void)
{
	struct phy_device *phydev;
	ofnode phy_node;

	phy_node = ofnode_get_phy_node(ofnode_path("ethernet0"));
	if (!ofnode_valid(phy_node))
		goto err;

	phydev = dm_phy_find_by_ofnode(phy_node);
	if (!phydev)
		goto err;

	mox_phy_modify(phydev, 3, 0x12, 0x700, 0x400);
	mox_phy_modify(phydev, 3, 0x10, 0xff, 0xbb);

	return;
err:
	printf("Cannot get ethernet PHY!\n");
}

static bool read_reset_button(void)
{
	struct udevice *button, *led;
	int i;

	if (device_get_global_by_ofnode(
			ofnode_first_subnode(ofnode_by_compatible(ofnode_null(),
								  "gpio-keys")),
			&button)) {
		printf("Cannot find reset button!\n");
		return false;
	}

	if (device_get_global_by_ofnode(
			ofnode_first_subnode(ofnode_by_compatible(ofnode_null(),
								  "gpio-leds")),
			&led)) {
		printf("Cannot find status LED!\n");
		return false;
	}

	led_set_state(led, LEDST_ON);

	for (i = 0; i < 21; ++i) {
		if (button_get_state(button) != BUTTON_ON)
			return false;
		if (i < 20)
			mdelay(50);
	}

	led_set_state(led, LEDST_OFF);

	return true;
}

static void handle_reset_button(void)
{
	const char * const vars[1] = { "bootcmd_rescue", };

	/*
	 * Ensure that bootcmd_rescue has always stock value, so that running
	 *   run bootcmd_rescue
	 * always works correctly.
	 */
	env_set_default_vars(1, (char * const *)vars, 0);

	if (read_reset_button()) {
		const char * const vars[3] = {
			"bootcmd",
			"bootdelay",
			"distro_bootcmd",
		};

		/*
		 * Set the above envs to their default values, in case the user
		 * managed to break them.
		 */
		env_set_default_vars(3, (char * const *)vars, 0);

		/* Ensure bootcmd_rescue is used by distroboot */
		env_set("boot_targets", "rescue");

		/* start blinking PHY LEDs */
		mox_phy_leds_start_blinking();

		printf("RESET button was pressed, overwriting boot_targets!\n");
	} else {
		/*
		 * In case the user somehow managed to save environment with
		 * boot_targets=rescue, reset boot_targets to default value.
		 * This could happen in subsequent commands if bootcmd_rescue
		 * failed.
		 */
		if (!strcmp(env_get("boot_targets"), "rescue")) {
			const char * const vars[1] = {
				"boot_targets",
			};

			env_set_default_vars(1, (char * const *)vars, 0);
		}
	}
}

int checkboard(void)
{
	int i, ret, board_version, ram_size, is_sd;
	const char *pub_key, *model;
	const u8 *topology;
	u64 serial_number;

	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("Model: %s\n", model);

	ret = mbox_sp_get_board_info(&serial_number, NULL, NULL, &board_version,
				     &ram_size, NULL);
	if (ret < 0) {
		printf("  Cannot read board info: %i\n", ret);
	} else {
		printf("  Board version: %i\n", board_version);
		printf("  RAM size: %i MiB\n", ram_size);
		printf("  Serial Number: %016llX\n", serial_number);
	}

	pub_key = mox_sp_get_ecdsa_public_key();
	if (pub_key)
		printf("  ECDSA Public Key: %s\n", pub_key);
	else
		printf("  Cannot read ECDSA Public Key\n");

	if (gd->board_type != BOARD_TURRIS_MOX)
		return 0;

	ret = mox_get_topology(&topology, &module_count, &is_sd);
	if (ret)
		printf("Cannot read module topology!\n");

	printf("  SD/eMMC version: %s\n", is_sd ? "SD" : "eMMC");

	if (module_count)
		printf("Module Topology:\n");

	for (i = 0; i < module_count; ++i) {
		switch (topology[i]) {
		case MOX_MODULE_SFP:
			printf("% 4i: SFP Module\n", i + 1);
			break;
		case MOX_MODULE_PCI:
			printf("% 4i: Mini-PCIe Module\n", i + 1);
			break;
		case MOX_MODULE_TOPAZ:
			printf("% 4i: Topaz Switch Module (4-port)\n", i + 1);
			break;
		case MOX_MODULE_PERIDOT:
			printf("% 4i: Peridot Switch Module (8-port)\n", i + 1);
			break;
		case MOX_MODULE_USB3:
			printf("% 4i: USB 3.0 Module (4 ports)\n", i + 1);
			break;
		case MOX_MODULE_PASSPCI:
			printf("% 4i: Passthrough Mini-PCIe Module\n", i + 1);
			break;
		default:
			printf("% 4i: unknown (ID %i)\n", i + 1, topology[i]);
		}
	}

	/* check if modules are connected in supported mode */
	for (i = 0; i < module_count; ++i) {
		switch (topology[i]) {
		case MOX_MODULE_SFP:
			if (sfp) {
				printf("Error: Only one SFP module is supported!\n");
			} else if (topaz) {
				printf("Error: SFP module cannot be connected after Topaz Switch module!\n");
			} else {
				sfp_pos = i;
				++sfp;
			}
			break;
		case MOX_MODULE_PCI:
			if (pci)
				printf("Error: Only one Mini-PCIe module is supported!\n");
			else if (usb)
				printf("Error: Mini-PCIe module cannot come after USB 3.0 module!\n");
			else if (i && (i != 1 || !passpci))
				printf("Error: Mini-PCIe module should be the first connected module or come right after Passthrough Mini-PCIe module!\n");
			else
				++pci;
			break;
		case MOX_MODULE_TOPAZ:
			if (topaz)
				printf("Error: Only one Topaz module is supported!\n");
			else if (peridot >= 3)
				printf("Error: At most two Peridot modules can come before Topaz module!\n");
			else
				++topaz;
			break;
		case MOX_MODULE_PERIDOT:
			if (sfp || topaz) {
				printf("Error: Peridot module must come before SFP or Topaz module!\n");
			} else if (peridot >= 3) {
				printf("Error: At most three Peridot modules are supported!\n");
			} else {
				peridot_pos[peridot] = i;
				++peridot;
			}
			break;
		case MOX_MODULE_USB3:
			if (pci)
				printf("Error: USB 3.0 module cannot come after Mini-PCIe module!\n");
			else if (usb)
				printf("Error: Only one USB 3.0 module is supported!\n");
			else if (i && (i != 1 || !passpci))
				printf("Error: USB 3.0 module should be the first connected module or come right after Passthrough Mini-PCIe module!\n");
			else
				++usb;
			break;
		case MOX_MODULE_PASSPCI:
			if (passpci)
				printf("Error: Only one Passthrough Mini-PCIe module is supported!\n");
			else if (i != 0)
				printf("Error: Passthrough Mini-PCIe module should be the first connected module!\n");
			else
				++passpci;
		}
	}

	return 0;
}

static struct udevice *mox_mdio_bus(void)
{
	struct udevice *bus;
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "marvell,orion-mdio");
	if (!ofnode_valid(node))
		goto err;

	dm_mdio_probe_devices();

	if (uclass_get_device_by_ofnode(UCLASS_MDIO, node, &bus))
		goto err;

	return bus;
err:
	printf("Cannot get MDIO bus device!\n");
	return NULL;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio > 0)
		return ENVL_UNKNOWN;

	if (gd->board_type == BOARD_RIPE_ATLAS)
		return ENVL_MMC;

	return ENVL_SPI_FLASH;
}

static int last_stage_init(void)
{
	struct gpio_desc reset_gpio = {};

	if (gd->board_type != BOARD_TURRIS_MOX)
		return 0;

	/* configure modules */
	if (get_reset_gpio(&reset_gpio) < 0)
		goto handle_reset_btn;

	if (peridot > 0) {
		if (configure_peridots(&reset_gpio) < 0) {
			printf("Cannot configure Peridot modules!\n");
			peridot = 0;
		}
	} else {
		dm_gpio_set_value(&reset_gpio, 1);
		mdelay(50);
		dm_gpio_set_value(&reset_gpio, 0);
		mdelay(50);
	}

	/*
	 * check if the addresses are set by reading Scratch & Misc register
	 * 0x70 of Peridot (and potentially Topaz) modules
	 */
	if (peridot || topaz) {
		struct udevice *bus = mox_mdio_bus();

		if (bus) {
			int i;

			for (i = 0; i < peridot; ++i)
				check_switch_address(bus, 0x10 + i);

			if (topaz)
				check_switch_address(bus, 0x2);

			sw_blink_leds(bus, peridot, topaz);
		}
	}

handle_reset_btn:
	handle_reset_button();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

#if defined(CONFIG_OF_BOARD_SETUP)

static bool is_topaz(int id)
{
	return topaz && id == peridot + topaz - 1;
}

static int switch_addr(int id)
{
	return is_topaz(id) ? 0x2 : 0x10 + id;
}

static int setup_switch(void *blob, int id)
{
	int res, addr, i, node;
	char mdio_path[64];

	node = fdt_node_offset_by_compatible(blob, -1, "marvell,orion-mdio");
	if (node < 0)
		return node;

	res = fdt_get_path(blob, node, mdio_path, sizeof(mdio_path));
	if (res < 0)
		return res;

	addr = switch_addr(id);

	/* first enable the switch by setting status = "okay" */
	res = fdt_status_okay_by_pathf(blob, "%s/switch%i@%x", mdio_path, id,
				       addr);
	if (res < 0)
		return res;

	/*
	 * now if there are more switches or a SFP module coming after,
	 * enable corresponding ports
	 */
	if (id < peridot + topaz - 1) {
		res = fdt_status_okay_by_pathf(blob,
					       "%s/switch%i@%x/ports/port@a",
					       mdio_path, id, addr);
	} else if (id == peridot - 1 && !topaz && sfp) {
		res = fdt_status_okay_by_pathf(blob,
					       "%s/switch%i@%x/ports/port-sfp@a",
					       mdio_path, id, addr);
	} else {
		res = 0;
	}
	if (res < 0)
		return res;

	if (id >= peridot + topaz - 1)
		return 0;

	/* finally change link property if needed */
	node = fdt_node_offset_by_pathf(blob, "%s/switch%i@%x/ports/port@a",
					mdio_path, id, addr);
	if (node < 0)
		return node;

	for (i = id + 1; i < peridot + topaz; ++i) {
		unsigned int phandle;

		phandle = fdt_create_phandle_by_pathf(blob,
						      "%s/switch%i@%x/ports/port@%x",
						      mdio_path, i,
						      switch_addr(i),
						      is_topaz(i) ? 5 : 9);
		if (!phandle)
			return -FDT_ERR_NOPHANDLES;

		if (i == id + 1)
			res = fdt_setprop_u32(blob, node, "link", phandle);
		else
			res = fdt_appendprop_u32(blob, node, "link", phandle);
		if (res < 0)
			return res;
	}

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int res;

	if (gd->board_type != BOARD_TURRIS_MOX)
		return 0;

	/*
	 * If MOX B (PCI), MOX F (USB) or MOX G (Passthrough PCI) modules are
	 * connected, enable the PCIe node.
	 */
	if (pci || usb || passpci) {
		res = fdt_status_okay_by_compatible(blob,
						    "marvell,armada-3700-pcie");
		if (res < 0)
			return res;

		/* Fix PCIe regions for devices with 4 GB RAM */
		res = a3700_fdt_fix_pcie_regions(blob);
		if (res < 0)
			return res;
	}

	/*
	 * If MOX C (Topaz switch) and/or MOX E (Peridot switch) are connected,
	 * enable the eth1 node and setup the switches.
	 */
	if (peridot || topaz) {
		int i;

		res = fdt_status_okay_by_alias(blob, "ethernet1");
		if (res < 0)
			return res;

		for (i = 0; i < peridot + topaz; ++i) {
			res = setup_switch(blob, i);
			if (res < 0)
				return res;
		}
	}

	/*
	 * If MOX D (SFP cage module) is connected, enable the SFP node and eth1
	 * node. If there is no Peridot switch between MOX A and MOX D, add link
	 * to the SFP node to eth1 node.
	 * Also enable and configure SFP GPIO controller node.
	 */
	if (sfp) {
		int node;

		res = fdt_status_okay_by_compatible(blob, "sff,sfp");
		if (res < 0)
			return res;

		res = fdt_status_okay_by_alias(blob, "ethernet1");
		if (res < 0)
			return res;

		if (!peridot) {
			unsigned int phandle;

			phandle = fdt_create_phandle_by_compatible(blob,
								   "sff,sfp");
			if (!phandle)
				return -FDT_ERR_NOPHANDLES;

			node = fdt_path_offset(blob, "ethernet1");
			if (node < 0)
				return node;

			res = fdt_setprop_u32(blob, node, "sfp", phandle);
			if (res < 0)
				return res;

			res = fdt_setprop_string(blob, node, "phy-mode",
						 "sgmii");
			if (res < 0)
				return res;

			res = fdt_setprop_string(blob, node, "label",
						 "sfp");
			if (res < 0)
				return res;
		}

		res = fdt_status_okay_by_compatible(blob, "cznic,moxtet-gpio");
		if (res < 0)
			return res;

		if (sfp_pos) {
			char newname[16];

			/* moxtet-sfp is on non-zero position, change default */
			node = fdt_node_offset_by_compatible(blob, -1,
							     "cznic,moxtet-gpio");
			if (node < 0)
				return node;

			res = fdt_setprop_u32(blob, node, "reg", sfp_pos);
			if (res < 0)
				return res;

			sprintf(newname, "gpio@%x", sfp_pos);

			res = fdt_set_name(blob, node, newname);
			if (res < 0)
				return res;
		}
	}

	fdt_fixup_ethernet(blob);

	/* Finally remove disabled nodes, as per Rob Herring's request. */
	fdt_delete_disabled_nodes(blob);

	return 0;
}

#endif
