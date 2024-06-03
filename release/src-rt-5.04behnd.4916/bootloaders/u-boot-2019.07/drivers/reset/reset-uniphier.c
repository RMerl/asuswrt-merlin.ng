// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

struct uniphier_reset_data {
	unsigned int id;
	unsigned int reg;
	unsigned int bit;
	unsigned int flags;
#define UNIPHIER_RESET_ACTIVE_LOW		BIT(0)
};

#define UNIPHIER_RESET_ID_END		(unsigned int)(-1)

#define UNIPHIER_RESET_END				\
	{ .id = UNIPHIER_RESET_ID_END }

#define UNIPHIER_RESET(_id, _reg, _bit)			\
	{						\
		.id = (_id),				\
		.reg = (_reg),				\
		.bit = (_bit),				\
	}

#define UNIPHIER_RESETX(_id, _reg, _bit)		\
	{						\
		.id = (_id),				\
		.reg = (_reg),				\
		.bit = (_bit),				\
		.flags = UNIPHIER_RESET_ACTIVE_LOW,	\
	}

/* System reset data */
static const struct uniphier_reset_data uniphier_pro4_sys_reset_data[] = {
	UNIPHIER_RESETX(2, 0x2000, 2),		/* NAND */
	UNIPHIER_RESETX(6, 0x2000, 12),		/* ETHER */
	UNIPHIER_RESETX(8, 0x2000, 10),		/* STDMAC */
	UNIPHIER_RESETX(12, 0x2000, 6),		/* GIO */
	UNIPHIER_RESETX(14, 0x2000, 17),	/* USB30 */
	UNIPHIER_RESETX(15, 0x2004, 17),	/* USB31 */
	UNIPHIER_RESET_END,
};

static const struct uniphier_reset_data uniphier_pxs2_sys_reset_data[] = {
	UNIPHIER_RESETX(2, 0x2000, 2),		/* NAND */
	UNIPHIER_RESETX(6, 0x2000, 12),		/* ETHER */
	UNIPHIER_RESETX(8, 0x2000, 10),		/* STDMAC */
	UNIPHIER_RESETX(14, 0x2000, 17),	/* USB30 */
	UNIPHIER_RESETX(15, 0x2004, 17),	/* USB31 */
	UNIPHIER_RESETX(16, 0x2014, 4),		/* USB30-PHY0 */
	UNIPHIER_RESETX(17, 0x2014, 0),		/* USB30-PHY1 */
	UNIPHIER_RESETX(18, 0x2014, 2),		/* USB30-PHY2 */
	UNIPHIER_RESETX(20, 0x2014, 5),		/* USB31-PHY0 */
	UNIPHIER_RESETX(21, 0x2014, 1),		/* USB31-PHY1 */
	UNIPHIER_RESETX(28, 0x2014, 12),	/* SATA */
	UNIPHIER_RESET(29, 0x2014, 8),		/* SATA-PHY (active high) */
	UNIPHIER_RESET_END,
};

static const struct uniphier_reset_data uniphier_ld20_sys_reset_data[] = {
	UNIPHIER_RESETX(2, 0x200c, 0),		/* NAND */
	UNIPHIER_RESETX(4, 0x200c, 2),		/* eMMC */
	UNIPHIER_RESETX(6, 0x200c, 6),		/* ETHER */
	UNIPHIER_RESETX(8, 0x200c, 8),		/* STDMAC */
	UNIPHIER_RESETX(14, 0x200c, 5),		/* USB30 */
	UNIPHIER_RESETX(16, 0x200c, 12),	/* USB30-PHY0 */
	UNIPHIER_RESETX(17, 0x200c, 13),	/* USB30-PHY1 */
	UNIPHIER_RESETX(18, 0x200c, 14),	/* USB30-PHY2 */
	UNIPHIER_RESETX(19, 0x200c, 15),	/* USB30-PHY3 */
	UNIPHIER_RESET_END,
};

static const struct uniphier_reset_data uniphier_pxs3_sys_reset_data[] = {
	UNIPHIER_RESETX(2, 0x200c, 0),		/* NAND */
	UNIPHIER_RESETX(4, 0x200c, 2),		/* eMMC */
	UNIPHIER_RESETX(6, 0x200c, 9),		/* ETHER0 */
	UNIPHIER_RESETX(7, 0x200c, 10),		/* ETHER1 */
	UNIPHIER_RESETX(8, 0x200c, 12),		/* STDMAC */
	UNIPHIER_RESETX(12, 0x200c, 4),		/* USB30 link */
	UNIPHIER_RESETX(13, 0x200c, 5),		/* USB31 link */
	UNIPHIER_RESETX(16, 0x200c, 16),	/* USB30-PHY0 */
	UNIPHIER_RESETX(17, 0x200c, 18),	/* USB30-PHY1 */
	UNIPHIER_RESETX(18, 0x200c, 20),	/* USB30-PHY2 */
	UNIPHIER_RESETX(20, 0x200c, 17),	/* USB31-PHY0 */
	UNIPHIER_RESETX(21, 0x200c, 19),	/* USB31-PHY1 */
	UNIPHIER_RESET_END,
};

/* Media I/O reset data */
#define UNIPHIER_MIO_RESET_SD(id, ch)			\
	UNIPHIER_RESETX((id), 0x110 + 0x200 * (ch), 0)

#define UNIPHIER_MIO_RESET_SD_BRIDGE(id, ch)		\
	UNIPHIER_RESETX((id), 0x110 + 0x200 * (ch), 26)

#define UNIPHIER_MIO_RESET_EMMC_HW_RESET(id, ch)	\
	UNIPHIER_RESETX((id), 0x80 + 0x200 * (ch), 0)

#define UNIPHIER_MIO_RESET_USB2(id, ch)			\
	UNIPHIER_RESETX((id), 0x114 + 0x200 * (ch), 0)

#define UNIPHIER_MIO_RESET_USB2_BRIDGE(id, ch)		\
	UNIPHIER_RESETX((id), 0x110 + 0x200 * (ch), 24)

#define UNIPHIER_MIO_RESET_DMAC(id)			\
	UNIPHIER_RESETX((id), 0x110, 17)

static const struct uniphier_reset_data uniphier_mio_reset_data[] = {
	UNIPHIER_MIO_RESET_SD(0, 0),
	UNIPHIER_MIO_RESET_SD(1, 1),
	UNIPHIER_MIO_RESET_SD(2, 2),
	UNIPHIER_MIO_RESET_SD_BRIDGE(3, 0),
	UNIPHIER_MIO_RESET_SD_BRIDGE(4, 1),
	UNIPHIER_MIO_RESET_SD_BRIDGE(5, 2),
	UNIPHIER_MIO_RESET_EMMC_HW_RESET(6, 1),
	UNIPHIER_MIO_RESET_DMAC(7),
	UNIPHIER_MIO_RESET_USB2(8, 0),
	UNIPHIER_MIO_RESET_USB2(9, 1),
	UNIPHIER_MIO_RESET_USB2(10, 2),
	UNIPHIER_MIO_RESET_USB2(11, 3),
	UNIPHIER_MIO_RESET_USB2_BRIDGE(12, 0),
	UNIPHIER_MIO_RESET_USB2_BRIDGE(13, 1),
	UNIPHIER_MIO_RESET_USB2_BRIDGE(14, 2),
	UNIPHIER_MIO_RESET_USB2_BRIDGE(15, 3),
	UNIPHIER_RESET_END,
};

/* Peripheral reset data */
#define UNIPHIER_PERI_RESET_UART(id, ch)		\
	UNIPHIER_RESETX((id), 0x114, 19 + (ch))

#define UNIPHIER_PERI_RESET_I2C(id, ch)			\
	UNIPHIER_RESETX((id), 0x114, 5 + (ch))

#define UNIPHIER_PERI_RESET_FI2C(id, ch)		\
	UNIPHIER_RESETX((id), 0x114, 24 + (ch))

static const struct uniphier_reset_data uniphier_ld4_peri_reset_data[] = {
	UNIPHIER_PERI_RESET_UART(0, 0),
	UNIPHIER_PERI_RESET_UART(1, 1),
	UNIPHIER_PERI_RESET_UART(2, 2),
	UNIPHIER_PERI_RESET_UART(3, 3),
	UNIPHIER_PERI_RESET_I2C(4, 0),
	UNIPHIER_PERI_RESET_I2C(5, 1),
	UNIPHIER_PERI_RESET_I2C(6, 2),
	UNIPHIER_PERI_RESET_I2C(7, 3),
	UNIPHIER_PERI_RESET_I2C(8, 4),
	UNIPHIER_RESET_END,
};

static const struct uniphier_reset_data uniphier_pro4_peri_reset_data[] = {
	UNIPHIER_PERI_RESET_UART(0, 0),
	UNIPHIER_PERI_RESET_UART(1, 1),
	UNIPHIER_PERI_RESET_UART(2, 2),
	UNIPHIER_PERI_RESET_UART(3, 3),
	UNIPHIER_PERI_RESET_FI2C(4, 0),
	UNIPHIER_PERI_RESET_FI2C(5, 1),
	UNIPHIER_PERI_RESET_FI2C(6, 2),
	UNIPHIER_PERI_RESET_FI2C(7, 3),
	UNIPHIER_PERI_RESET_FI2C(8, 4),
	UNIPHIER_PERI_RESET_FI2C(9, 5),
	UNIPHIER_PERI_RESET_FI2C(10, 6),
	UNIPHIER_RESET_END,
};

/* core implementaton */
struct uniphier_reset_priv {
	void __iomem *base;
	const struct uniphier_reset_data *data;
};

static int uniphier_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int uniphier_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int uniphier_reset_update(struct reset_ctl *reset_ctl, int assert)
{
	struct uniphier_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	unsigned long id = reset_ctl->id;
	const struct uniphier_reset_data *p;

	for (p = priv->data; p->id != UNIPHIER_RESET_ID_END; p++) {
		u32 mask, val;

		if (p->id != id)
			continue;

		val = readl(priv->base + p->reg);

		if (p->flags & UNIPHIER_RESET_ACTIVE_LOW)
			assert = !assert;

		mask = BIT(p->bit);

		if (assert)
			val |= mask;
		else
			val &= ~mask;

		writel(val, priv->base + p->reg);

		return 0;
	}

	dev_err(reset_ctl->dev, "reset_id=%lu was not handled\n", id);

	return -EINVAL;
}

static int uniphier_reset_assert(struct reset_ctl *reset_ctl)
{
	return uniphier_reset_update(reset_ctl, 1);
}

static int uniphier_reset_deassert(struct reset_ctl *reset_ctl)
{
	return uniphier_reset_update(reset_ctl, 0);
}

static const struct reset_ops uniphier_reset_ops = {
	.request = uniphier_reset_request,
	.free = uniphier_reset_free,
	.rst_assert = uniphier_reset_assert,
	.rst_deassert = uniphier_reset_deassert,
};

static int uniphier_reset_probe(struct udevice *dev)
{
	struct uniphier_reset_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_4K);
	if (!priv->base)
		return -ENOMEM;

	priv->data = (void *)dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id uniphier_reset_match[] = {
	/* System reset */
	{
		.compatible = "socionext,uniphier-ld4-reset",
		.data = (ulong)uniphier_pro4_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-reset",
		.data = (ulong)uniphier_pro4_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-reset",
		.data = (ulong)uniphier_pro4_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-reset",
		.data = (ulong)uniphier_pro4_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-reset",
		.data = (ulong)uniphier_pxs2_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-reset",
		.data = (ulong)uniphier_ld20_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-reset",
		.data = (ulong)uniphier_ld20_sys_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-reset",
		.data = (ulong)uniphier_pxs3_sys_reset_data,
	},
	/* Media I/O reset */
	{
		.compatible = "socionext,uniphier-ld4-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-mio-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-sd-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-sd-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-sd-reset",
		.data = (ulong)uniphier_mio_reset_data,
	},
	/* Peripheral reset */
	{
		.compatible = "socionext,uniphier-ld4-peri-reset",
		.data = (ulong)uniphier_ld4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-peri-reset",
		.data = (ulong)uniphier_ld4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-peri-reset",
		.data = (ulong)uniphier_pro4_peri_reset_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_reset) = {
	.name = "uniphier-reset",
	.id = UCLASS_RESET,
	.of_match = uniphier_reset_match,
	.probe = uniphier_reset_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_reset_priv),
	.ops = &uniphier_reset_ops,
};
