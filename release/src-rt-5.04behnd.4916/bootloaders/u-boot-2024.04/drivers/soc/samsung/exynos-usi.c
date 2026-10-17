// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Samsung Exynos USI driver (Universal Serial Interface).
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <dt-bindings/soc/samsung,exynos-usi.h>

/* USIv2: System Register: SW_CONF register bits */
#define USI_V2_SW_CONF_NONE	0x0
#define USI_V2_SW_CONF_UART	BIT(0)
#define USI_V2_SW_CONF_SPI	BIT(1)
#define USI_V2_SW_CONF_I2C	BIT(2)
#define USI_V2_SW_CONF_MASK	(USI_V2_SW_CONF_UART | USI_V2_SW_CONF_SPI | \
				 USI_V2_SW_CONF_I2C)

/* USIv2: USI register offsets */
#define USI_CON			0x04
#define USI_OPTION		0x08

/* USIv2: USI register bits */
#define USI_CON_RESET		BIT(0)
#define USI_OPTION_CLKREQ_ON	BIT(1)
#define USI_OPTION_CLKSTOP_ON	BIT(2)

enum exynos_usi_ver {
	USI_VER2 = 2,
};

struct exynos_usi_variant {
	enum exynos_usi_ver ver;	/* USI IP-core version */
	unsigned int sw_conf_mask;	/* SW_CONF mask for all protocols */
	size_t min_mode;		/* first index in exynos_usi_modes[] */
	size_t max_mode;		/* last index in exynos_usi_modes[] */
};

struct exynos_usi {
	void __iomem *regs;		/* USI register map */

	size_t mode;			/* current USI SW_CONF mode index */
	bool clkreq_on;			/* always provide clock to IP */

	/* System Register */
	struct regmap *sysreg;		/* System Register map */
	unsigned int sw_conf;		/* SW_CONF register offset in sysreg */

	const struct exynos_usi_variant *data;
};

struct exynos_usi_mode {
	const char *name;		/* mode name */
	unsigned int val;		/* mode register value */
};

static const struct exynos_usi_mode exynos_usi_modes[] = {
	[USI_V2_NONE] =	{ .name = "none", .val = USI_V2_SW_CONF_NONE },
	[USI_V2_UART] =	{ .name = "uart", .val = USI_V2_SW_CONF_UART },
	[USI_V2_SPI] =	{ .name = "spi",  .val = USI_V2_SW_CONF_SPI },
	[USI_V2_I2C] =	{ .name = "i2c",  .val = USI_V2_SW_CONF_I2C },
};

static const struct exynos_usi_variant exynos850_usi_data = {
	.ver		= USI_VER2,
	.sw_conf_mask	= USI_V2_SW_CONF_MASK,
	.min_mode	= USI_V2_NONE,
	.max_mode	= USI_V2_I2C,
};

static const struct udevice_id exynos_usi_ids[] = {
	{
		.compatible = "samsung,exynos850-usi",
		.data = (ulong)&exynos850_usi_data,
	},
	{ } /* sentinel */
};

/**
 * exynos_usi_set_sw_conf - Set USI block configuration mode
 * @dev: Driver object
 *
 * Select underlying serial protocol (UART/SPI/I2C) in USI IP-core as specified
 * in @usi.mode.
 *
 * Return: 0 on success, or negative error code on failure.
 */
static int exynos_usi_set_sw_conf(struct udevice *dev)
{
	struct exynos_usi *usi = dev_get_priv(dev);
	size_t mode = usi->mode;
	unsigned int val;
	int ret;

	if (mode < usi->data->min_mode || mode > usi->data->max_mode)
		return -EINVAL;

	val = exynos_usi_modes[mode].val;
	ret = regmap_update_bits(usi->sysreg, usi->sw_conf,
				 usi->data->sw_conf_mask, val);
	if (ret)
		return ret;

	dev_dbg(dev, "protocol: %s\n", exynos_usi_modes[mode].name);

	return 0;
}

/**
 * exynos_usi_enable - Initialize USI block
 * @usi: USI driver object
 *
 * USI IP-core start state is "reset" (on startup and after CPU resume). This
 * routine enables the USI block by clearing the reset flag. It also configures
 * HWACG behavior (needed e.g. for UART Rx). It should be performed before
 * underlying protocol becomes functional.
 */
static void exynos_usi_enable(const struct exynos_usi *usi)
{
	u32 val;

	/* Enable USI block */
	val = readl(usi->regs + USI_CON);
	val &= ~USI_CON_RESET;
	writel(val, usi->regs + USI_CON);
	udelay(1);

	/* Continuously provide the clock to USI IP w/o gating */
	if (usi->clkreq_on) {
		val = readl(usi->regs + USI_OPTION);
		val &= ~USI_OPTION_CLKSTOP_ON;
		val |= USI_OPTION_CLKREQ_ON;
		writel(val, usi->regs + USI_OPTION);
	}
}

static int exynos_usi_configure(struct udevice *dev)
{
	struct exynos_usi *usi = dev_get_priv(dev);
	int ret;

	ret = exynos_usi_set_sw_conf(dev);
	if (ret)
		return ret;

	if (usi->data->ver == USI_VER2)
		exynos_usi_enable(usi);

	return 0;
}

static int exynos_usi_of_to_plat(struct udevice *dev)
{
	struct exynos_usi *usi = dev_get_priv(dev);
	ofnode node = dev_ofnode(dev);
	int ret;
	u32 mode;

	usi->data = (struct exynos_usi_variant *)dev_get_driver_data(dev);
	if (usi->data->ver == USI_VER2) {
		usi->regs = dev_read_addr_ptr(dev);
		if (!usi->regs)
			return -ENODEV;
	}

	ret = ofnode_read_u32(node, "samsung,mode", &mode);
	if (ret)
		return ret;
	if (mode < usi->data->min_mode || mode > usi->data->max_mode)
		return -EINVAL;
	usi->mode = mode;

	usi->sysreg = syscon_regmap_lookup_by_phandle(dev, "samsung,sysreg");
	if (IS_ERR(usi->sysreg))
		return PTR_ERR(usi->sysreg);

	ret = ofnode_read_u32_index(node, "samsung,sysreg", 1, &usi->sw_conf);
	if (ret)
		return ret;

	usi->clkreq_on = ofnode_read_bool(node, "samsung,clkreq-on");

	return 0;
}

static int exynos_usi_probe(struct udevice *dev)
{
	return exynos_usi_configure(dev);
}

U_BOOT_DRIVER(exynos_usi) = {
	.name		= "exynos-usi",
	.id		= UCLASS_MISC,
	.of_match	= exynos_usi_ids,
	.of_to_plat	= exynos_usi_of_to_plat,
	.probe		= exynos_usi_probe,
	.priv_auto	= sizeof(struct exynos_usi),
};
