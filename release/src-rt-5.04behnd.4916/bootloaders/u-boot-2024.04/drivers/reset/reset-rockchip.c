// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
/*
 * Each reg has 16 bits reset signal for devices
 * Note: Not including rk2818 and older SoCs
 */
#define ROCKCHIP_RESET_NUM_IN_REG	16

struct rockchip_reset_priv {
	void __iomem *base;
	const int *lut;
	/* Rockchip reset reg locate at cru controller */
	u32 reset_reg_offset;
	/* Rockchip reset reg number */
	u32 reset_reg_num;
};

static int rockchip_reset_request(struct reset_ctl *reset_ctl)
{
	struct rockchip_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	unsigned long id = reset_ctl->id;

	if (priv->lut)
		id = priv->lut[id];

	debug("%s(reset_ctl=%p) (dev=%p, id=%lu) (reg_num=%d)\n", __func__,
	      reset_ctl, reset_ctl->dev, id, priv->reset_reg_num);

	if (id / ROCKCHIP_RESET_NUM_IN_REG >= priv->reset_reg_num)
		return -EINVAL;

	return 0;
}

static int rockchip_reset_assert(struct reset_ctl *reset_ctl)
{
	struct rockchip_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	unsigned long id = reset_ctl->id;
	int bank, offset;

	if (priv->lut)
		id = priv->lut[id];

	bank = id / ROCKCHIP_RESET_NUM_IN_REG;
	offset = id % ROCKCHIP_RESET_NUM_IN_REG;

	debug("%s(reset_ctl=%p) (dev=%p, id=%lu) (reg_addr=%p)\n", __func__,
	      reset_ctl, reset_ctl->dev, id, priv->base + (bank * 4));

	rk_setreg(priv->base + (bank * 4), BIT(offset));

	return 0;
}

static int rockchip_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct rockchip_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	unsigned long id = reset_ctl->id;
	int bank, offset;

	if (priv->lut)
		id = priv->lut[id];

	bank = id / ROCKCHIP_RESET_NUM_IN_REG;
	offset = id % ROCKCHIP_RESET_NUM_IN_REG;

	debug("%s(reset_ctl=%p) (dev=%p, id=%lu) (reg_addr=%p)\n", __func__,
	      reset_ctl, reset_ctl->dev, id, priv->base + (bank * 4));

	rk_clrreg(priv->base + (bank * 4), BIT(offset));

	return 0;
}

struct reset_ops rockchip_reset_ops = {
	.request = rockchip_reset_request,
	.rst_assert = rockchip_reset_assert,
	.rst_deassert = rockchip_reset_deassert,
};

static int rockchip_reset_probe(struct udevice *dev)
{
	struct rockchip_reset_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = dev_read_addr_size(dev, &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	if ((priv->reset_reg_offset == 0) && (priv->reset_reg_num == 0))
		return -EINVAL;

	addr += priv->reset_reg_offset;
	priv->base = ioremap(addr, size);

	debug("%s(base=%p) (reg_offset=%x, reg_num=%d)\n", __func__,
	      priv->base, priv->reset_reg_offset, priv->reset_reg_num);

	return 0;
}

int rockchip_reset_bind_lut(struct udevice *pdev,
			    const int *lookup_table,
			    u32 reg_offset,
			    u32 reg_number)
{
	struct udevice *rst_dev;
	struct rockchip_reset_priv *priv;
	int ret;

	ret = device_bind_driver_to_node(pdev, "rockchip_reset", "reset",
					 dev_ofnode(pdev), &rst_dev);
	if (ret) {
		debug("Warning: No rockchip reset driver: ret=%d\n", ret);
		return ret;
	}
	priv = malloc(sizeof(struct rockchip_reset_priv));
	priv->reset_reg_offset = reg_offset;
	priv->reset_reg_num = reg_number;
	priv->lut = lookup_table;
	dev_set_priv(rst_dev, priv);

	return 0;
}

int rockchip_reset_bind(struct udevice *pdev, u32 reg_offset, u32 reg_number)
{
	return rockchip_reset_bind_lut(pdev, NULL, reg_offset, reg_number);
}

U_BOOT_DRIVER(rockchip_reset) = {
	.name = "rockchip_reset",
	.id = UCLASS_RESET,
	.probe = rockchip_reset_probe,
	.ops = &rockchip_reset_ops,
	.priv_auto	= sizeof(struct rockchip_reset_priv),
};
