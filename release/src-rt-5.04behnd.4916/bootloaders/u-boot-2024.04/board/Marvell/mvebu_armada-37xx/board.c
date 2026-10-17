// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <env.h>
#include <env_internal.h>
#include <event.h>
#include <i2c.h>
#include <init.h>
#include <mmc.h>
#include <miiphy.h>
#include <phy.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* IO expander I2C device */
#define I2C_IO_EXP_ADDR		0x22
#define I2C_IO_CFG_REG_0	0x6
#define I2C_IO_DATA_OUT_REG_0	0x2
#define I2C_IO_REG_0_SATA_OFF	2
#define I2C_IO_REG_0_USB_H_OFF	1

/* The pin control values are the same for DB and Espressobin */
#define PINCTRL_NB_REG_VALUE	0x000173fa
#define PINCTRL_SB_REG_VALUE	0x00007a23

/* Ethernet switch registers */
/* SMI addresses for multi-chip mode */
#define MVEBU_PORT_CTRL_SMI_ADDR(p)	(16 + (p))
#define MVEBU_SW_G2_SMI_ADDR		(28)

/* Multi-chip mode */
#define MVEBU_SW_SMI_DATA_REG		(1)
#define MVEBU_SW_SMI_CMD_REG		(0)
 #define SW_SMI_CMD_REG_ADDR_OFF	0
 #define SW_SMI_CMD_DEV_ADDR_OFF	5
 #define SW_SMI_CMD_SMI_OP_OFF		10
 #define SW_SMI_CMD_SMI_MODE_OFF	12
 #define SW_SMI_CMD_SMI_BUSY_OFF	15

/* Single-chip mode */
/* Switch Port Registers */
#define MVEBU_SW_LINK_CTRL_REG		(1)
#define MVEBU_SW_PORT_SWITCH_ID		(3)
#define MVEBU_SW_PORT_CTRL_REG		(4)
#define MVEBU_SW_PORT_BASE_VLAN		(6)

/* Global 2 Registers */
#define MVEBU_G2_SMI_PHY_CMD_REG	(24)
#define MVEBU_G2_SMI_PHY_DATA_REG	(25)

#define SWITCH_88E6361_PRODUCT_NUMBER	0x2610

/*
 * Memory Controller Registers
 *
 * Assembled based on public information:
 * https://gitlab.nic.cz/turris/mox-boot-builder/-/blob/v2020.11.26/wtmi/main.c#L332-336
 * https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell/blob/mv_ddr-armada-18.12/drivers/mv_ddr_mc6.h#L309-L332
 *
 * And checked against the written register values for the various topologies:
 * https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell/blob/master/a3700/mv_ddr_tim.h
 */
#define A3700_CH0_MC_CTRL2_REG		MVEBU_REGISTER(0x002c4)
#define A3700_MC_CTRL2_SDRAM_TYPE_MASK	0xf
#define A3700_MC_CTRL2_SDRAM_TYPE_OFFS	4
#define A3700_MC_CTRL2_SDRAM_TYPE_DDR3	2
#define A3700_MC_CTRL2_SDRAM_TYPE_DDR4	3

static bool is_edpu_plus(void)
{
	struct udevice *bus;
	ofnode node;
	int val;

	if (!CONFIG_IS_ENABLED(DM_MDIO))
		return false;

	node = ofnode_by_compatible(ofnode_null(), "marvell,orion-mdio");
	if (!ofnode_valid(node) ||
	    uclass_get_device_by_ofnode(UCLASS_MDIO, node, &bus) ||
	    device_probe(bus)) {
		printf("Cannot find MDIO bus\n");
		return -ENODEV;
	}

	val = dm_mdio_read(bus, 0x0, MDIO_DEVAD_NONE, MVEBU_SW_PORT_SWITCH_ID);
	if (val == SWITCH_88E6361_PRODUCT_NUMBER)
		return true;
	else
		return false;
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	char *ptr = &default_environment[0];
	struct udevice *dev;
	struct mmc *mmc_dev;
	bool ddr4, emmc;
	const char *mac;
	char eth[10];
	int i;

	if (!of_machine_is_compatible("globalscale,espressobin"))
		return 0;

	/*
	 * Find free space for new variables in default_environment[] array.
	 * Free space is after the last variable, each variable is termined
	 * by nul byte and after the last variable is additional nul byte.
	 * Move ptr to the position where new variable can be filled.
	 */
	while (*ptr != '\0') {
		do { ptr++; } while (*ptr != '\0');
		ptr++;
	}

	/*
	 * Ensure that 'env default -a' does not erase permanent MAC addresses
	 * stored in env variables: $ethaddr, $eth1addr, $eth2addr and $eth3addr
	 */

	mac = env_get("ethaddr");
	if (mac && strlen(mac) <= 17)
		ptr += sprintf(ptr, "ethaddr=%s", mac) + 1;

	for (i = 1; i <= 3; i++) {
		sprintf(eth, "eth%daddr", i);
		mac = env_get(eth);
		if (mac && strlen(mac) <= 17)
			ptr += sprintf(ptr, "%s=%s", eth, mac) + 1;
	}

	/* If the memory controller has been configured for DDR4, we're running on v7 */
	ddr4 = ((readl(A3700_CH0_MC_CTRL2_REG) >> A3700_MC_CTRL2_SDRAM_TYPE_OFFS)
		& A3700_MC_CTRL2_SDRAM_TYPE_MASK) == A3700_MC_CTRL2_SDRAM_TYPE_DDR4;

	/* eMMC is mmc dev num 1 */
	mmc_dev = find_mmc_device(1);
	emmc = (mmc_dev && mmc_get_op_cond(mmc_dev, true) == 0);

	/* if eMMC is not present then remove it from DM */
	if (!emmc && mmc_dev) {
		dev = mmc_dev->dev;
		device_remove(dev, DM_REMOVE_NORMAL);
		device_unbind(dev);
		if (of_live_active())
			ofnode_set_enabled(dev_ofnode(dev), false);
	}

	/* Ensure that 'env default -a' set correct value to $fdtfile */
	if (ddr4 && emmc)
		strcpy(ptr, "fdtfile=marvell/armada-3720-espressobin-v7-emmc.dtb");
	else if (ddr4)
		strcpy(ptr, "fdtfile=marvell/armada-3720-espressobin-v7.dtb");
	else if (emmc)
		strcpy(ptr, "fdtfile=marvell/armada-3720-espressobin-emmc.dtb");
	else
		strcpy(ptr, "fdtfile=marvell/armada-3720-espressobin.dtb");
	ptr += strlen(ptr) + 1;

	/*
	 * After the last variable (which is nul term string) append another nul
	 * byte which terminates the list. So everything after ptr is ignored.
	 */
	*ptr = '\0';

	return 0;
}
#endif

/* Board specific AHCI / SATA enable code */
int board_ahci_enable(void)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	/* Only DB requres this configuration */
	if (!of_machine_is_compatible("marvell,armada-3720-db"))
		return 0;

	/* Configure IO exander PCA9555: 7bit address 0x22 */
	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
	if (ret) {
		printf("Cannot find PCA9555: %d\n", ret);
		return 0;
	}

	ret = dm_i2c_read(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}

	/*
	 * Enable SATA power via IO expander connected via I2C by setting
	 * the corresponding bit to output mode to enable power for SATA
	 */
	buf[0] &= ~(1 << I2C_IO_REG_0_SATA_OFF);
	ret = dm_i2c_write(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	return 0;
}

/* Board specific xHCI enable code */
int board_xhci_enable(fdt_addr_t base)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	/* Only DB requres this configuration */
	if (!of_machine_is_compatible("marvell,armada-3720-db"))
		return 0;

	/* Configure IO exander PCA9555: 7bit address 0x22 */
	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
	if (ret) {
		printf("Cannot find PCA9555: %d\n", ret);
		return 0;
	}

	printf("Enable USB VBUS\n");

	/*
	 * Read configuration (direction) and set VBUS pin as output
	 * (reset pin = output)
	 */
	ret = dm_i2c_read(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}
	buf[0] &= ~(1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	/* Read VBUS output value and disable it */
	ret = dm_i2c_read(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}
	buf[0] &= ~(1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	/*
	 * Required delay for configuration to settle - must wait for
	 * power on port is disabled in case VBUS signal was high,
	 * required 3 seconds delay to let VBUS signal fully settle down
	 */
	mdelay(3000);

	/* Enable VBUS power: Set output value of VBUS pin as enabled */
	buf[0] |= (1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	mdelay(500); /* required delay to let output value settle */

	return 0;
}

#ifdef CONFIG_LAST_STAGE_INIT
/* Helper function for accessing switch devices in multi-chip connection mode */
static int mii_multi_chip_mode_write(struct udevice *bus, int dev_smi_addr,
				     int smi_addr, int reg, u16 value)
{
	u16 smi_cmd = 0;

	if (dm_mdio_write(bus, dev_smi_addr, MDIO_DEVAD_NONE,
			  MVEBU_SW_SMI_DATA_REG, value) != 0) {
		printf("Error writing to the PHY addr=%02x reg=%02x\n",
		       smi_addr, reg);
		return -EFAULT;
	}

	smi_cmd = (1 << SW_SMI_CMD_SMI_BUSY_OFF) |
		  (1 << SW_SMI_CMD_SMI_MODE_OFF) |
		  (1 << SW_SMI_CMD_SMI_OP_OFF) |
		  (smi_addr << SW_SMI_CMD_DEV_ADDR_OFF) |
		  (reg << SW_SMI_CMD_REG_ADDR_OFF);
	if (dm_mdio_write(bus, dev_smi_addr, MDIO_DEVAD_NONE,
			  MVEBU_SW_SMI_CMD_REG, smi_cmd) != 0) {
		printf("Error writing to the PHY addr=%02x reg=%02x\n",
		       smi_addr, reg);
		return -EFAULT;
	}

	return 0;
}

static int espressobin_last_stage_init(void)
{
	struct udevice *bus;
	ofnode node;

	if (!CONFIG_IS_ENABLED(DM_MDIO))
		return 0;

	node = ofnode_by_compatible(ofnode_null(), "marvell,orion-mdio");
	if (!ofnode_valid(node) ||
	    uclass_get_device_by_ofnode(UCLASS_MDIO, node, &bus) ||
	    device_probe(bus)) {
		printf("Cannot find MDIO bus\n");
		return 0;
	}

	/*
	 * FIXME: remove this code once Topaz driver gets available
	 * A3720 Community Board Only
	 * Configure Topaz switch (88E6341)
	 * Restrict output to ports 1,2,3 only from port 0 (CPU)
	 * Set port 0,1,2,3 to forwarding Mode (through Switch Port registers)
	 */
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(1),
				  MVEBU_SW_PORT_BASE_VLAN, BIT(0));
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(2),
				  MVEBU_SW_PORT_BASE_VLAN, BIT(0));
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(3),
				  MVEBU_SW_PORT_BASE_VLAN, BIT(0));

	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(0),
				  MVEBU_SW_PORT_CTRL_REG, 0x7f);
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(1),
				  MVEBU_SW_PORT_CTRL_REG, 0x7f);
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(2),
				  MVEBU_SW_PORT_CTRL_REG, 0x7f);
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(3),
				  MVEBU_SW_PORT_CTRL_REG, 0x7f);

	/* RGMII Delay on Port 0 (CPU port), force link to 1000Mbps */
	mii_multi_chip_mode_write(bus, 1, MVEBU_PORT_CTRL_SMI_ADDR(0),
				  MVEBU_SW_LINK_CTRL_REG, 0xe002);

	/* Power up PHY 1, 2, 3 (through Global 2 registers) */
	mii_multi_chip_mode_write(bus, 1, MVEBU_SW_G2_SMI_ADDR,
				  MVEBU_G2_SMI_PHY_DATA_REG, 0x1140);
	mii_multi_chip_mode_write(bus, 1, MVEBU_SW_G2_SMI_ADDR,
				  MVEBU_G2_SMI_PHY_CMD_REG, 0x9620);
	mii_multi_chip_mode_write(bus, 1, MVEBU_SW_G2_SMI_ADDR,
				  MVEBU_G2_SMI_PHY_CMD_REG, 0x9640);
	mii_multi_chip_mode_write(bus, 1, MVEBU_SW_G2_SMI_ADDR,
				  MVEBU_G2_SMI_PHY_CMD_REG, 0x9660);

	return 0;
}

static int edpu_plus_last_stage_init(void)
{
	struct udevice *dev;
	int ret;

	if (is_edpu_plus()) {
		ret = uclass_get_device_by_name(UCLASS_ETH,
						"ethernet@40000",
						&dev);
		if (!ret) {
			device_remove(dev, DM_REMOVE_NORMAL);
			device_unbind(dev);
		}

		/* Currently no networking support on the eDPU+ board */
		ret = uclass_get_device_by_name(UCLASS_ETH,
						"ethernet@30000",
						&dev);
		if (!ret) {
			device_remove(dev, DM_REMOVE_NORMAL);
			device_unbind(dev);
		}
	} else {
		ret = uclass_get_device_by_name(UCLASS_ETH,
						"ethernet@30000",
						&dev);
		if (!ret) {
			device_remove(dev, DM_REMOVE_NORMAL);
			device_unbind(dev);
		}
	}

	return 0;
}

/* Bring-up board-specific network stuff */
static int last_stage_init(void)
{

	if (of_machine_is_compatible("globalscale,espressobin"))
		return espressobin_last_stage_init();

	if (of_machine_is_compatible("methode,edpu"))
		return edpu_plus_last_stage_init();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
#endif

#ifdef CONFIG_OF_BOARD_SETUP
static int espressobin_fdt_setup(void *blob)
{
	int ret;
	int spi_off;
	int parts_off;
	int part_off;

	/* Fill SPI MTD partitions for Linux kernel on Espressobin */
	spi_off = fdt_node_offset_by_compatible(blob, -1, "jedec,spi-nor");
	if (spi_off < 0)
		return 0;

	/* Do not touch partitions if they are already defined */
	if (fdt_subnode_offset(blob, spi_off, "partitions") >= 0)
		return 0;

	parts_off = fdt_add_subnode(blob, spi_off, "partitions");
	if (parts_off < 0) {
		printf("Can't add partitions node: %s\n", fdt_strerror(parts_off));
		return 0;
	}

	ret = fdt_setprop_string(blob, parts_off, "compatible", "fixed-partitions");
	if (ret < 0) {
		printf("Can't set compatible property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_setprop_u32(blob, parts_off, "#address-cells", 1);
	if (ret < 0) {
		printf("Can't set #address-cells property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_setprop_u32(blob, parts_off, "#size-cells", 1);
	if (ret < 0) {
		printf("Can't set #size-cells property: %s\n", fdt_strerror(ret));
		return 0;
	}

	/* Add u-boot-env partition */

	part_off = fdt_add_subnode(blob, parts_off, "partition@u-boot-env");
	if (part_off < 0) {
		printf("Can't add partition@u-boot-env node: %s\n", fdt_strerror(part_off));
		return 0;
	}

	ret = fdt_setprop_u32(blob, part_off, "reg", CONFIG_ENV_OFFSET);
	if (ret < 0) {
		printf("Can't set partition@u-boot-env reg property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_appendprop_u32(blob, part_off, "reg", CONFIG_ENV_SIZE);
	if (ret < 0) {
		printf("Can't set partition@u-boot-env reg property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_setprop_string(blob, part_off, "label", "u-boot-env");
	if (ret < 0) {
		printf("Can't set partition@u-boot-env label property: %s\n", fdt_strerror(ret));
		return 0;
	}

	/* Add firmware partition */

	part_off = fdt_add_subnode(blob, parts_off, "partition@firmware");
	if (part_off < 0) {
		printf("Can't add partition@firmware node: %s\n", fdt_strerror(part_off));
		return 0;
	}

	ret = fdt_setprop_u32(blob, part_off, "reg", 0);
	if (ret < 0) {
		printf("Can't set partition@firmware reg property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_appendprop_u32(blob, part_off, "reg", CONFIG_ENV_OFFSET);
	if (ret < 0) {
		printf("Can't set partition@firmware reg property: %s\n", fdt_strerror(ret));
		return 0;
	}

	ret = fdt_setprop_string(blob, part_off, "label", "firmware");
	if (ret < 0) {
		printf("Can't set partition@firmware label property: %s\n", fdt_strerror(ret));
		return 0;
	}

	return 0;
}

static int edpu_plus_fdt_setup(void *blob)
{
	const char *ports[] = { "downlink", "uplink" };
	uint8_t mac[ETH_ALEN];
	const char *path;
	int i, ret;

	if (is_edpu_plus()) {
		ret = fdt_set_status_by_compatible(blob,
						   "marvell,orion-mdio",
						   FDT_STATUS_OKAY);
		if (ret)
			printf("Failed to enable MDIO!\n");

		ret = fdt_set_status_by_alias(blob,
					      "ethernet1",
					      FDT_STATUS_DISABLED);
		if (ret)
			printf("Failed to disable ethernet1!\n");

		path = fdt_get_alias(blob, "ethernet0");
		if (path)
			do_fixup_by_path_string(blob, path, "phy-mode", "2500base-x");
		else
			printf("Failed to update ethernet0 phy-mode to 2500base-x!\n");

		ret = fdt_set_status_by_compatible(blob,
						   "marvell,mv88e6190",
						   FDT_STATUS_OKAY);
		if (ret)
			printf("Failed to enable MV88E6361!\n");

		/*
		 * MAC-s for Uplink and Downlink ports are stored under
		 * non standard variable names, so lets manually fixup the
		 * switch port nodes to have the desired MAC-s.
		 */
		for (i = 0; i < 2; i++) {
			if (eth_env_get_enetaddr(ports[i], mac)) {
				do_fixup_by_prop(blob,
						 "label",
						 ports[i],
						 strlen(ports[i]) + 1,
						 "mac-address",
						 mac, ARP_HLEN, 1);

				do_fixup_by_prop(blob,
						 "label",
						 ports[i],
						 strlen(ports[i]) + 1,
						 "local-mac-address",
						 mac, ARP_HLEN, 1);
			}
		}
	}

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	if (of_machine_is_compatible("globalscale,espressobin"))
		return espressobin_fdt_setup(blob);
#endif
	if (of_machine_is_compatible("methode,edpu"))
		return edpu_plus_fdt_setup(blob);

	return 0;
}
#endif
