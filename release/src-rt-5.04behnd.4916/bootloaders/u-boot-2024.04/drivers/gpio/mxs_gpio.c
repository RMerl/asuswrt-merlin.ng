// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 GPIO control code
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>

#if	defined(CONFIG_MX23)
#define	PINCTRL_BANKS		3
#define	PINCTRL_DOUT(n)		(0x0500 + ((n) * 0x10))
#define	PINCTRL_DIN(n)		(0x0600 + ((n) * 0x10))
#define	PINCTRL_DOE(n)		(0x0700 + ((n) * 0x10))
#define	PINCTRL_PIN2IRQ(n)	(0x0800 + ((n) * 0x10))
#define	PINCTRL_IRQEN(n)	(0x0900 + ((n) * 0x10))
#define	PINCTRL_IRQSTAT(n)	(0x0c00 + ((n) * 0x10))
#elif	defined(CONFIG_MX28)
#define	PINCTRL_BANKS		5
#define	PINCTRL_DOUT(n)		(0x0700 + ((n) * 0x10))
#define	PINCTRL_DIN(n)		(0x0900 + ((n) * 0x10))
#define	PINCTRL_DOE(n)		(0x0b00 + ((n) * 0x10))
#define	PINCTRL_PIN2IRQ(n)	(0x1000 + ((n) * 0x10))
#define	PINCTRL_IRQEN(n)	(0x1100 + ((n) * 0x10))
#define	PINCTRL_IRQSTAT(n)	(0x1400 + ((n) * 0x10))
#else
#error "Please select CONFIG_MX23 or CONFIG_MX28"
#endif

#define GPIO_INT_FALL_EDGE	0x0
#define GPIO_INT_LOW_LEV	0x1
#define GPIO_INT_RISE_EDGE	0x2
#define GPIO_INT_HIGH_LEV	0x3
#define GPIO_INT_LEV_MASK	(1 << 0)
#define GPIO_INT_POL_MASK	(1 << 1)

void mxs_gpio_init(void)
{
	int i;

	for (i = 0; i < PINCTRL_BANKS; i++) {
		writel(0, MXS_PINCTRL_BASE + PINCTRL_PIN2IRQ(i));
		writel(0, MXS_PINCTRL_BASE + PINCTRL_IRQEN(i));
		/* Use SCT address here to clear the IRQSTAT bits */
		writel(0xffffffff, MXS_PINCTRL_BASE + PINCTRL_IRQSTAT(i) + 8);
	}
}

#if !CONFIG_IS_ENABLED(DM_GPIO)
int gpio_get_value(unsigned gpio)
{
	uint32_t bank = PAD_BANK(gpio);
	uint32_t offset = PINCTRL_DIN(bank);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE + offset);

	return (readl(&reg->reg) >> PAD_PIN(gpio)) & 1;
}

void gpio_set_value(unsigned gpio, int value)
{
	uint32_t bank = PAD_BANK(gpio);
	uint32_t offset = PINCTRL_DOUT(bank);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE + offset);

	if (value)
		writel(1 << PAD_PIN(gpio), &reg->reg_set);
	else
		writel(1 << PAD_PIN(gpio), &reg->reg_clr);
}

int gpio_direction_input(unsigned gpio)
{
	uint32_t bank = PAD_BANK(gpio);
	uint32_t offset = PINCTRL_DOE(bank);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE + offset);

	writel(1 << PAD_PIN(gpio), &reg->reg_clr);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	uint32_t bank = PAD_BANK(gpio);
	uint32_t offset = PINCTRL_DOE(bank);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE + offset);

	gpio_set_value(gpio, value);

	writel(1 << PAD_PIN(gpio), &reg->reg_set);

	return 0;
}

int gpio_request(unsigned gpio, const char *label)
{
	if (PAD_BANK(gpio) >= PINCTRL_BANKS)
		return -1;

	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int name_to_gpio(const char *name)
{
	unsigned bank, pin;
	char *end;

	bank = dectoul(name, &end);

	if (!*end || *end != ':')
		return bank;

	pin = dectoul(end + 1, NULL);

	return (bank << MXS_PAD_BANK_SHIFT) | (pin << MXS_PAD_PIN_SHIFT);
}
#else /* DM_GPIO */
#include <dm.h>
#include <asm/gpio.h>
#include <dt-structs.h>
#include <asm/arch/gpio.h>
#define MXS_MAX_GPIO_PER_BANK		32

DECLARE_GLOBAL_DATA_PTR;
/*
 * According to i.MX28 Reference Manual:
 * 'i.MX28 Applications Processor Reference Manual, Rev. 1, 2010'
 * The i.MX28 has following number of GPIOs available:
 * Bank 0: 0-28 -> 29 PINS
 * Bank 1: 0-31 -> 32 PINS
 * Bank 2: 0-27 -> 28 PINS
 * Bank 3: 0-30 -> 31 PINS
 * Bank 4: 0-20 -> 21 PINS
 */

struct mxs_gpio_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_gpio dtplat;
#endif
	unsigned int bank;
	int gpio_ranges;
};

struct mxs_gpio_priv {
	unsigned int bank;
};

static int mxs_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE +
					   PINCTRL_DIN(priv->bank));

	return (readl(&reg->reg) >> offset) & 1;
}

static int mxs_gpio_set_value(struct udevice *dev, unsigned offset,
			      int value)
{
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE +
					   PINCTRL_DOUT(priv->bank));
	if (value)
		writel(BIT(offset), &reg->reg_set);
	else
		writel(BIT(offset), &reg->reg_clr);

	return 0;
}

static int mxs_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE +
					   PINCTRL_DOE(priv->bank));

	writel(BIT(offset), &reg->reg_clr);

	return 0;
}

static int mxs_gpio_direction_output(struct udevice *dev, unsigned offset,
				     int value)
{
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE +
					   PINCTRL_DOE(priv->bank));

	mxs_gpio_set_value(dev, offset, value);

	writel(BIT(offset), &reg->reg_set);

	return 0;
}

static int mxs_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct mxs_register_32 *reg =
		(struct mxs_register_32 *)(MXS_PINCTRL_BASE +
					   PINCTRL_DOE(priv->bank));
	bool is_output = !!(readl(&reg->reg) >> offset);

	return is_output ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_mxs_ops = {
	.direction_input	= mxs_gpio_direction_input,
	.direction_output	= mxs_gpio_direction_output,
	.get_value		= mxs_gpio_get_value,
	.set_value		= mxs_gpio_set_value,
	.get_function		= mxs_gpio_get_function,
};

static int mxs_gpio_probe(struct udevice *dev)
{
	struct mxs_gpio_plat *plat = dev_get_plat(dev);
	struct mxs_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[16], *str;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_gpio *dtplat = &plat->dtplat;
	priv->bank = (unsigned int)dtplat->reg[0];
	uc_priv->gpio_count = dtplat->gpio_ranges[3];
#else
	priv->bank = (unsigned int)plat->bank;
	uc_priv->gpio_count = plat->gpio_ranges;
#endif
	snprintf(name, sizeof(name), "GPIO%d_", priv->bank);
	str = strdup(name);
	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;

	debug("%s: %s: %d pins base: 0x%x\n", __func__, uc_priv->bank_name,
	      uc_priv->gpio_count, priv->bank);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_REAL)
static int mxs_of_to_plat(struct udevice *dev)
{
	struct mxs_gpio_plat *plat = dev_get_plat(dev);
	struct fdtdec_phandle_args args;
	int node = dev_of_offset(dev);
	int ret;

	plat->bank = dev_read_addr(dev);
	if (plat->bank == FDT_ADDR_T_NONE) {
		printf("%s: No 'reg' property defined!\n", __func__);
		return -EINVAL;
	}

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, node, "gpio-ranges",
					     NULL, 3, 0, &args);
	if (ret)
		printf("%s: 'gpio-ranges' not defined - using default!\n",
		       __func__);

	plat->gpio_ranges = ret == 0 ? args.args[2] : MXS_MAX_GPIO_PER_BANK;

	return 0;
}

static const struct udevice_id mxs_gpio_ids[] = {
	{ .compatible = "fsl,imx23-gpio" },
	{ .compatible = "fsl,imx28-gpio" },
	{ }
};
#endif

U_BOOT_DRIVER(fsl_imx23_gpio) = {
	.name = "fsl_imx23_gpio",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mxs_ops,
	.probe	= mxs_gpio_probe,
	.priv_auto	= sizeof(struct mxs_gpio_priv),
	.plat_auto	= sizeof(struct mxs_gpio_plat),
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = mxs_gpio_ids,
	.of_to_plat = mxs_of_to_plat,
#endif
};

DM_DRIVER_ALIAS(fsl_imx23_gpio, fsl_imx28_gpio)
#endif /* DM_GPIO */
