// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <phy.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

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
#define MVEBU_SW_PORT_CTRL_REG		(4)

/* Global 2 Registers */
#define MVEBU_G2_SMI_PHY_CMD_REG	(24)
#define MVEBU_G2_SMI_PHY_DATA_REG	(25)

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

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

/* Helper function for accessing switch devices in multi-chip connection mode */
static int mii_multi_chip_mode_write(struct mii_dev *bus, int dev_smi_addr,
				     int smi_addr, int reg, u16 value)
{
	u16 smi_cmd = 0;

	if (bus->write(bus, dev_smi_addr, 0,
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
	if (bus->write(bus, dev_smi_addr, 0,
		       MVEBU_SW_SMI_CMD_REG, smi_cmd) != 0) {
		printf("Error writing to the PHY addr=%02x reg=%02x\n",
		       smi_addr, reg);
		return -EFAULT;
	}

	return 0;
}

/* Bring-up board-specific network stuff */
int board_network_enable(struct mii_dev *bus)
{
	if (!of_machine_is_compatible("marvell,armada-3720-espressobin"))
		return 0;

	/*
	 * FIXME: remove this code once Topaz driver gets available
	 * A3720 Community Board Only
	 * Configure Topaz switch (88E6341)
	 * Set port 0,1,2,3 to forwarding Mode (through Switch Port registers)
	 */
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
