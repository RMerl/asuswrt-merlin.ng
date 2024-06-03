// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs serial gpio driver
 *
 * Author: <lars.povlsen@microchip.com>
 *
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>
#include <clk.h>

#define MSCC_SGPIOS_PER_BANK	32
#define MSCC_SGPIO_BANK_DEPTH	4

enum {
	REG_INPUT_DATA,
	REG_PORT_CONFIG,
	REG_PORT_ENABLE,
	REG_SIO_CONFIG,
	REG_SIO_CLOCK,
	MAXREG
};

struct mscc_sgpio_bf {
	u8 beg;
	u8 end;
};

struct mscc_sgpio_props {
	u8 regoff[MAXREG];
	struct mscc_sgpio_bf auto_repeat;
	struct mscc_sgpio_bf port_width;
	struct mscc_sgpio_bf clk_freq;
	struct mscc_sgpio_bf bit_source;
};

#define __M(bf)		GENMASK((bf).end, (bf).beg)
#define __F(bf, x)	(__M(bf) & ((x) << (bf).beg))
#define __X(bf, x)	(((x) >> (bf).beg) & GENMASK(((bf).end - (bf).beg), 0))

#define MSCC_M_CFG_SIO_AUTO_REPEAT(p)		BIT(p->props->auto_repeat.beg)
#define MSCC_F_CFG_SIO_PORT_WIDTH(p, x)		__F(p->props->port_width, x)
#define MSCC_M_CFG_SIO_PORT_WIDTH(p)		__M(p->props->port_width)
#define MSCC_F_CLOCK_SIO_CLK_FREQ(p, x)		__F(p->props->clk_freq, x)
#define MSCC_M_CLOCK_SIO_CLK_FREQ(p)		__M(p->props->clk_freq)
#define MSCC_F_PORT_CFG_BIT_SOURCE(p, x)	__F(p->props->bit_source, x)
#define MSCC_X_PORT_CFG_BIT_SOURCE(p, x)	__X(p->props->bit_source, x)

const struct mscc_sgpio_props props_luton = {
	.regoff = { 0x00, 0x09, 0x29, 0x2a, 0x2b },
	.auto_repeat = { 5, 5 },
	.port_width  = { 2, 3 },
	.clk_freq    = { 0, 11 },
	.bit_source  = { 0, 11 },
};

const struct mscc_sgpio_props props_ocelot = {
	.regoff = { 0x00, 0x06, 0x26, 0x04, 0x05 },
	.auto_repeat = { 10, 10 },
	.port_width  = {  7, 8  },
	.clk_freq    = {  8, 19 },
	.bit_source  = { 12, 23 },
};

struct mscc_sgpio_priv {
	u32 bitcount;
	u32 ports;
	u32 clock;
	u32 mode[MSCC_SGPIOS_PER_BANK];
	u32 __iomem *regs;
	const struct mscc_sgpio_props *props;
};

static inline u32 sgpio_readl(struct mscc_sgpio_priv *priv, u32 rno, u32 off)
{
	u32 __iomem *reg = &priv->regs[priv->props->regoff[rno] + off];

	return readl(reg);
}

static inline void sgpio_writel(struct mscc_sgpio_priv *priv,
				u32 val, u32 rno, u32 off)
{
	u32 __iomem *reg = &priv->regs[priv->props->regoff[rno] + off];

	writel(val, reg);
}

static void sgpio_clrsetbits(struct mscc_sgpio_priv *priv,
			     u32 rno, u32 off, u32 clear, u32 set)
{
	u32 __iomem *reg = &priv->regs[priv->props->regoff[rno] + off];

	clrsetbits_le32(reg, clear, set);
}

static int mscc_sgpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct mscc_sgpio_priv *priv = dev_get_priv(dev);

	u32 port = gpio % MSCC_SGPIOS_PER_BANK;
	u32 bit = gpio / MSCC_SGPIOS_PER_BANK;

	priv->mode[port] |= BIT(bit);

	return 0;
}

static int mscc_sgpio_direction_output(struct udevice *dev,
				       unsigned int gpio, int value)
{
	struct mscc_sgpio_priv *priv = dev_get_priv(dev);
	u32 port = gpio % MSCC_SGPIOS_PER_BANK;
	u32 bit = gpio / MSCC_SGPIOS_PER_BANK;
	u32 mask = 3 << (3 * bit);

	debug("set: port %d, bit %d, mask 0x%08x, value %d\n",
	      port, bit, mask, value);

	value = (value & 3) << (3 * bit);
	sgpio_clrsetbits(priv, REG_PORT_CONFIG, port,
			 MSCC_F_PORT_CFG_BIT_SOURCE(priv, mask),
			 MSCC_F_PORT_CFG_BIT_SOURCE(priv, value));
	clrbits_le32(&priv->mode[port], BIT(bit));

	return 0;
}

static int mscc_sgpio_get_function(struct udevice *dev, unsigned int gpio)
{
	struct mscc_sgpio_priv *priv = dev_get_priv(dev);
	u32 port = gpio % MSCC_SGPIOS_PER_BANK;
	u32 bit = gpio / MSCC_SGPIOS_PER_BANK;
	u32 val = priv->mode[port] & BIT(bit);

	if (val)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static int mscc_sgpio_set_value(struct udevice *dev,
				unsigned int gpio, int value)
{
	return mscc_sgpio_direction_output(dev, gpio, value);
}

static int mscc_sgpio_get_value(struct udevice *dev, unsigned int gpio)
{
	struct mscc_sgpio_priv *priv = dev_get_priv(dev);
	u32 port = gpio % MSCC_SGPIOS_PER_BANK;
	u32 bit = gpio / MSCC_SGPIOS_PER_BANK;
	int ret;

	if (mscc_sgpio_get_function(dev, gpio) == GPIOF_INPUT) {
		ret = !!(sgpio_readl(priv, REG_INPUT_DATA, bit) & BIT(port));
	} else {
		u32 portval = sgpio_readl(priv, REG_PORT_CONFIG, port);

		ret = MSCC_X_PORT_CFG_BIT_SOURCE(priv, portval);
		ret = !!(ret & (3 << (3 * bit)));
	}

	debug("get: gpio %d, port %d, bit %d, value %d\n",
	      gpio, port, bit, ret);
	return ret;
}

static int mscc_sgpio_get_count(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	int count = 0, i = 0, ret;

	ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3, i, &args);
	while (ret != -ENOENT) {
		count += args.args[2];
		ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3,
						 ++i, &args);
	}
	return count;
}

static int mscc_sgpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mscc_sgpio_priv *priv = dev_get_priv(dev);
	int err, div_clock = 0, port;
	u32 val;
	struct clk clk;

	err = clk_get_by_index(dev, 0, &clk);
	if (!err) {
		err = clk_get_rate(&clk);
		if (IS_ERR_VALUE(err)) {
			dev_err(dev, "Invalid clk rate\n");
			return -EINVAL;
		}
		div_clock = err;
	} else {
		dev_err(dev, "Failed to get clock\n");
		return err;
	}

	priv->props = (const struct mscc_sgpio_props *)dev_get_driver_data(dev);
	priv->ports = dev_read_u32_default(dev, "mscc,sgpio-ports", 0xFFFFFFFF);
	priv->clock = dev_read_u32_default(dev, "mscc,sgpio-frequency",
					   12500000);
	if (priv->clock <= 0 || priv->clock > div_clock) {
		dev_err(dev, "Invalid frequency %d\n", priv->clock);
		return -EINVAL;
	}

	uc_priv->gpio_count = mscc_sgpio_get_count(dev);
	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios",
						   uc_priv->gpio_count);
	if (uc_priv->gpio_count < 1 || uc_priv->gpio_count >
	    (4 * MSCC_SGPIOS_PER_BANK)) {
		dev_err(dev, "Invalid gpio count %d\n", uc_priv->gpio_count);
		return -EINVAL;
	}
	priv->bitcount = DIV_ROUND_UP(uc_priv->gpio_count,
				      MSCC_SGPIOS_PER_BANK);
	debug("probe: gpios = %d, bit-count = %d\n",
	      uc_priv->gpio_count, priv->bitcount);

	priv->regs = (u32 __iomem *)dev_read_addr(dev);
	uc_priv->bank_name = "sgpio";

	sgpio_clrsetbits(priv, REG_SIO_CONFIG, 0,
			 MSCC_M_CFG_SIO_PORT_WIDTH(priv),
			 MSCC_F_CFG_SIO_PORT_WIDTH(priv, priv->bitcount - 1) |
			 MSCC_M_CFG_SIO_AUTO_REPEAT(priv));
	val = div_clock / priv->clock;
	debug("probe: div-clock = %d KHz, freq = %d KHz, div = %d\n",
	      div_clock / 1000, priv->clock / 1000, val);
	sgpio_clrsetbits(priv, REG_SIO_CLOCK, 0,
			 MSCC_M_CLOCK_SIO_CLK_FREQ(priv),
			 MSCC_F_CLOCK_SIO_CLK_FREQ(priv, val));

	for (port = 0; port < 32; port++)
		sgpio_writel(priv, 0, REG_PORT_CONFIG, port);
	sgpio_writel(priv, priv->ports, REG_PORT_ENABLE, 0);

	debug("probe: sgpio regs = %p\n", priv->regs);

	return 0;
}

static const struct dm_gpio_ops mscc_sgpio_ops = {
	.direction_input	= mscc_sgpio_direction_input,
	.direction_output	= mscc_sgpio_direction_output,
	.get_function		= mscc_sgpio_get_function,
	.get_value		= mscc_sgpio_get_value,
	.set_value		= mscc_sgpio_set_value,
};

static const struct udevice_id mscc_sgpio_ids[] = {
	{ .compatible = "mscc,luton-sgpio", .data = (ulong)&props_luton },
	{ .compatible = "mscc,ocelot-sgpio", .data = (ulong)&props_ocelot },
	{ }
};

U_BOOT_DRIVER(gpio_mscc_sgpio) = {
	.name			= "mscc-sgpio",
	.id			= UCLASS_GPIO,
	.of_match		= mscc_sgpio_ids,
	.ops			= &mscc_sgpio_ops,
	.probe			= mscc_sgpio_probe,
	.priv_auto_alloc_size	= sizeof(struct mscc_sgpio_priv),
};
