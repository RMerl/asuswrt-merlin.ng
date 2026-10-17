// SPDX-License-Identifier: GPL-2.0+
/*
 * HSDK SoC Reset Controller driver
 *
 * Copyright (C) 2019 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#include <log.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <reset-uclass.h>

struct hsdk_rst {
	void __iomem		*regs_ctl;
	void __iomem		*regs_rst;
};

static const u32 rst_map[] = {
	BIT(16), /* APB_RST  */
	BIT(17), /* AXI_RST  */
	BIT(18), /* ETH_RST  */
	BIT(19), /* USB_RST  */
	BIT(20), /* SDIO_RST */
	BIT(21), /* HDMI_RST */
	BIT(22), /* GFX_RST  */
	BIT(25), /* DMAC_RST */
	BIT(31), /* EBI_RST  */
};

#define HSDK_MAX_RESETS			ARRAY_SIZE(rst_map)

#define CGU_SYS_RST_CTRL		0x0
#define CGU_IP_SW_RESET			0x0
#define CGU_IP_SW_RESET_DELAY_SHIFT	16
#define CGU_IP_SW_RESET_DELAY_MASK	GENMASK(31, CGU_IP_SW_RESET_DELAY_SHIFT)
#define CGU_IP_SW_RESET_DELAY		0
#define CGU_IP_SW_RESET_RESET		BIT(0)
#define SW_RESET_TIMEOUT		10000

static void hsdk_reset_config(struct hsdk_rst *rst, unsigned long id)
{
	writel(rst_map[id], rst->regs_ctl + CGU_SYS_RST_CTRL);
}

static int hsdk_reset_do(struct hsdk_rst *rst)
{
	u32 reg;

	reg = readl(rst->regs_rst + CGU_IP_SW_RESET);
	reg &= ~CGU_IP_SW_RESET_DELAY_MASK;
	reg |= CGU_IP_SW_RESET_DELAY << CGU_IP_SW_RESET_DELAY_SHIFT;
	reg |= CGU_IP_SW_RESET_RESET;
	writel(reg, rst->regs_rst + CGU_IP_SW_RESET);

	/* wait till reset bit is back to 0 */
	return readl_poll_timeout(rst->regs_rst + CGU_IP_SW_RESET, reg,
		!(reg & CGU_IP_SW_RESET_RESET), SW_RESET_TIMEOUT);
}

static int hsdk_reset_reset(struct reset_ctl *rst_ctl)
{
	struct udevice *dev = rst_ctl->dev;
	struct hsdk_rst *rst = dev_get_priv(dev);

	if (rst_ctl->id >= HSDK_MAX_RESETS)
		return -EINVAL;

	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, rst_ctl,
	      rst_ctl->dev, rst_ctl->id);

	hsdk_reset_config(rst, rst_ctl->id);
	return hsdk_reset_do(rst);
}

static const struct reset_ops hsdk_reset_ops = {
	.rst_deassert	= hsdk_reset_reset,
};

static const struct udevice_id hsdk_reset_dt_match[] = {
	{ .compatible = "snps,hsdk-reset" },
	{ },
};

static int hsdk_reset_probe(struct udevice *dev)
{
	struct hsdk_rst *rst = dev_get_priv(dev);

	rst->regs_ctl = dev_remap_addr_index(dev, 0);
	if (!rst->regs_ctl)
		return -EINVAL;

	rst->regs_rst = dev_remap_addr_index(dev, 1);
	if (!rst->regs_rst)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(hsdk_reset) = {
	.name = "hsdk-reset",
	.id = UCLASS_RESET,
	.of_match = hsdk_reset_dt_match,
	.ops = &hsdk_reset_ops,
	.probe = hsdk_reset_probe,
	.priv_auto	= sizeof(struct hsdk_rst),
};
