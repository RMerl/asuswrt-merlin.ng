// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm IPQ4019 MDIO driver
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Luka Kovacic <luka.kovacic@sartura.hr>
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 * Based on Linux driver
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <miiphy.h>
#include <phy.h>

#define MDIO_MODE_REG               0x40
#define MDIO_ADDR_REG               0x44
#define MDIO_DATA_WRITE_REG         0x48
#define MDIO_DATA_READ_REG          0x4c
#define MDIO_CMD_REG                0x50
#define MDIO_CMD_ACCESS_BUSY        BIT(16)
#define MDIO_CMD_ACCESS_START       BIT(8)
#define MDIO_CMD_ACCESS_CODE_READ   0
#define MDIO_CMD_ACCESS_CODE_WRITE  1

/* 0 = Clause 22, 1 = Clause 45 */
#define MDIO_MODE_BIT               BIT(8)

#define IPQ4019_MDIO_TIMEOUT    10000
#define IPQ4019_MDIO_SLEEP      10

struct ipq4019_mdio_priv {
	phys_addr_t mdio_base;
};

static int ipq4019_mdio_wait_busy(struct ipq4019_mdio_priv *priv)
{
	unsigned int busy;

	return readl_poll_sleep_timeout(priv->mdio_base + MDIO_CMD_REG, busy,
				  (busy & MDIO_CMD_ACCESS_BUSY) == 0, IPQ4019_MDIO_SLEEP,
				  IPQ4019_MDIO_TIMEOUT);
}

int ipq4019_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct ipq4019_mdio_priv *priv = dev_get_priv(dev);
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	/* Issue the phy address and reg */
	writel((addr << 8) | reg, priv->mdio_base + MDIO_ADDR_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_READ;

	/* Issue read command */
	writel(cmd, priv->mdio_base + MDIO_CMD_REG);

	/* Wait read complete */
	if (ipq4019_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	/* Read and return data */
	return readl(priv->mdio_base + MDIO_DATA_READ_REG);
}

int ipq4019_mdio_write(struct udevice *dev, int addr, int devad,
					  int reg, u16 val)
{
	struct ipq4019_mdio_priv *priv = dev_get_priv(dev);
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	/* Issue the phy addreass and reg */
	writel((addr << 8) | reg, priv->mdio_base + MDIO_ADDR_REG);

	/* Issue write data */
	writel(val, priv->mdio_base + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_WRITE;

	/* Issue write command */
	writel(cmd, priv->mdio_base + MDIO_CMD_REG);

	/* Wait for write complete */

	if (ipq4019_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	return 0;
}

static const struct mdio_ops ipq4019_mdio_ops = {
	.read = ipq4019_mdio_read,
	.write = ipq4019_mdio_write,
};

static int ipq4019_mdio_bind(struct udevice *dev)
{
	if (ofnode_valid(dev_ofnode(dev)))
		device_set_name(dev, ofnode_get_name(dev_ofnode(dev)));

	return 0;
}

static int ipq4019_mdio_probe(struct udevice *dev)
{
	struct ipq4019_mdio_priv *priv = dev_get_priv(dev);
	unsigned int data;

	priv->mdio_base = dev_read_addr(dev);
	if (priv->mdio_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Enter Clause 22 mode */
	data = readl(priv->mdio_base + MDIO_MODE_REG);
	data &= ~MDIO_MODE_BIT;
	writel(data, priv->mdio_base + MDIO_MODE_REG);

	return 0;
}

static const struct udevice_id ipq4019_mdio_ids[] = {
	{ .compatible = "qcom,ipq4019-mdio", },
	{ }
};

U_BOOT_DRIVER(ipq4019_mdio) = {
	.name           = "ipq4019_mdio",
	.id             = UCLASS_MDIO,
	.of_match       = ipq4019_mdio_ids,
	.bind           = ipq4019_mdio_bind,
	.probe          = ipq4019_mdio_probe,
	.ops            = &ipq4019_mdio_ops,
	.priv_auto	  = sizeof(struct ipq4019_mdio_priv),
};
