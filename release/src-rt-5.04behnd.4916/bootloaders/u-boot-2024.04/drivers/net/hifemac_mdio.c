// SPDX-License-Identifier: GPL-2.0+
/*
 * Hisilicon Fast Ethernet MDIO Bus Driver
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 */

#include <dm.h>
#include <clk.h>
#include <miiphy.h>
#include <linux/io.h>
#include <linux/iopoll.h>

#define MDIO_RWCTRL		0x00
#define MDIO_RO_DATA		0x04
#define MDIO_WRITE		BIT(13)
#define MDIO_RW_FINISH		BIT(15)
#define BIT_PHY_ADDR_OFFSET	8
#define BIT_WR_DATA_OFFSET	16

struct hisi_femac_mdio_data {
	struct clk *clk;
	void __iomem *membase;
};

static int hisi_femac_mdio_wait_ready(struct hisi_femac_mdio_data *data)
{
	u32 val;

	return readl_poll_timeout(data->membase + MDIO_RWCTRL,
				  val, val & MDIO_RW_FINISH, 10000);
}

static int hisi_femac_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct hisi_femac_mdio_data *data = dev_get_priv(dev);
	int ret;

	ret = hisi_femac_mdio_wait_ready(data);
	if (ret)
		return ret;

	writel((addr << BIT_PHY_ADDR_OFFSET) | reg,
	       data->membase + MDIO_RWCTRL);

	ret = hisi_femac_mdio_wait_ready(data);
	if (ret)
		return ret;

	return readl(data->membase + MDIO_RO_DATA) & 0xFFFF;
}

static int hisi_femac_mdio_write(struct udevice *dev, int addr, int devad, int reg, u16 val)
{
	struct hisi_femac_mdio_data *data = dev_get_priv(dev);
	int ret;

	ret = hisi_femac_mdio_wait_ready(data);
	if (ret)
		return ret;

	writel(MDIO_WRITE | (val << BIT_WR_DATA_OFFSET) |
	       (addr << BIT_PHY_ADDR_OFFSET) | reg,
	       data->membase + MDIO_RWCTRL);

	return hisi_femac_mdio_wait_ready(data);
}

static int hisi_femac_mdio_of_to_plat(struct udevice *dev)
{
	struct hisi_femac_mdio_data *data = dev_get_priv(dev);
	int ret;

	data->membase = dev_remap_addr(dev);
	if (IS_ERR(data->membase)) {
		ret = PTR_ERR(data->membase);
		return log_msg_ret("Failed to remap base addr", ret);
	}

	// clk is optional
	data->clk = devm_clk_get_optional(dev, NULL);

	return 0;
}

static int hisi_femac_mdio_probe(struct udevice *dev)
{
	struct hisi_femac_mdio_data *data = dev_get_priv(dev);
	int ret;

	ret = clk_prepare_enable(data->clk);
	if (ret)
		return log_msg_ret("Failed to enable clk", ret);

	return 0;
}

static const struct mdio_ops hisi_femac_mdio_ops = {
	.read = hisi_femac_mdio_read,
	.write = hisi_femac_mdio_write,
};

static const struct udevice_id hisi_femac_mdio_dt_ids[] = {
	{ .compatible = "hisilicon,hisi-femac-mdio" },
	{ }
};

U_BOOT_DRIVER(hisi_femac_mdio_driver) = {
	.name = "hisi-femac-mdio",
	.id = UCLASS_MDIO,
	.of_match = hisi_femac_mdio_dt_ids,
	.of_to_plat = hisi_femac_mdio_of_to_plat,
	.probe = hisi_femac_mdio_probe,
	.ops = &hisi_femac_mdio_ops,
	.priv_auto = sizeof(struct hisi_femac_mdio_data),
};
