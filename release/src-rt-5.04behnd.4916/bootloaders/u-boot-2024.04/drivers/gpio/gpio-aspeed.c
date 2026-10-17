// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2015 IBM Corp.
 * Joel Stanley <joel@jms.id.au>
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * Implementation extracted from the Linux kernel and adapted for u-boot.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <config.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/sizes.h>

struct aspeed_gpio_priv {
	void *regs;
};

struct aspeed_gpio_bank {
	u16		val_regs;	/* +0: Rd: read input value, Wr: set write latch
					 * +4: Rd/Wr: Direction (0=in, 1=out)
					 */
	u16		rdata_reg;	/*     Rd: read write latch, Wr: <none>  */
	u16		irq_regs;
	u16		debounce_regs;
	u16		tolerance_regs;
	u16		cmdsrc_regs;
	const char	names[4][3];
};

static const struct aspeed_gpio_bank aspeed_gpio_banks[] = {
	{
		.val_regs = 0x0000,
		.rdata_reg = 0x00c0,
		.irq_regs = 0x0008,
		.debounce_regs = 0x0040,
		.tolerance_regs = 0x001c,
		.cmdsrc_regs = 0x0060,
		.names = { "A", "B", "C", "D" },
	},
	{
		.val_regs = 0x0020,
		.rdata_reg = 0x00c4,
		.irq_regs = 0x0028,
		.debounce_regs = 0x0048,
		.tolerance_regs = 0x003c,
		.cmdsrc_regs = 0x0068,
		.names = { "E", "F", "G", "H" },
	},
	{
		.val_regs = 0x0070,
		.rdata_reg = 0x00c8,
		.irq_regs = 0x0098,
		.debounce_regs = 0x00b0,
		.tolerance_regs = 0x00ac,
		.cmdsrc_regs = 0x0090,
		.names = { "I", "J", "K", "L" },
	},
	{
		.val_regs = 0x0078,
		.rdata_reg = 0x00cc,
		.irq_regs = 0x00e8,
		.debounce_regs = 0x0100,
		.tolerance_regs = 0x00fc,
		.cmdsrc_regs = 0x00e0,
		.names = { "M", "N", "O", "P" },
	},
	{
		.val_regs = 0x0080,
		.rdata_reg = 0x00d0,
		.irq_regs = 0x0118,
		.debounce_regs = 0x0130,
		.tolerance_regs = 0x012c,
		.cmdsrc_regs = 0x0110,
		.names = { "Q", "R", "S", "T" },
	},
	{
		.val_regs = 0x0088,
		.rdata_reg = 0x00d4,
		.irq_regs = 0x0148,
		.debounce_regs = 0x0160,
		.tolerance_regs = 0x015c,
		.cmdsrc_regs = 0x0140,
		.names = { "U", "V", "W", "X" },
	},
	{
		.val_regs = 0x01E0,
		.rdata_reg = 0x00d8,
		.irq_regs = 0x0178,
		.debounce_regs = 0x0190,
		.tolerance_regs = 0x018c,
		.cmdsrc_regs = 0x0170,
		.names = { "Y", "Z", "AA", "AB" },
	},
	{
		.val_regs = 0x01e8,
		.rdata_reg = 0x00dc,
		.irq_regs = 0x01a8,
		.debounce_regs = 0x01c0,
		.tolerance_regs = 0x01bc,
		.cmdsrc_regs = 0x01a0,
		.names = { "AC", "", "", "" },
	},
};

enum aspeed_gpio_reg {
	reg_val,
	reg_rdata,
	reg_dir,
	reg_irq_enable,
	reg_irq_type0,
	reg_irq_type1,
	reg_irq_type2,
	reg_irq_status,
	reg_debounce_sel1,
	reg_debounce_sel2,
	reg_tolerance,
	reg_cmdsrc0,
	reg_cmdsrc1,
};

#define GPIO_VAL_VALUE	0x00
#define GPIO_VAL_DIR	0x04

#define GPIO_IRQ_ENABLE	0x00
#define GPIO_IRQ_TYPE0	0x04
#define GPIO_IRQ_TYPE1	0x08
#define GPIO_IRQ_TYPE2	0x0c
#define GPIO_IRQ_STATUS	0x10

#define GPIO_DEBOUNCE_SEL1 0x00
#define GPIO_DEBOUNCE_SEL2 0x04

#define GPIO_CMDSRC_0	0x00
#define GPIO_CMDSRC_1	0x04
#define  GPIO_CMDSRC_ARM		0
#define  GPIO_CMDSRC_LPC		1
#define  GPIO_CMDSRC_COLDFIRE		2
#define  GPIO_CMDSRC_RESERVED		3

/* This will be resolved at compile time */
static inline void __iomem *bank_reg(struct aspeed_gpio_priv *gpio,
				     const struct aspeed_gpio_bank *bank,
				     const enum aspeed_gpio_reg reg)
{
	switch (reg) {
	case reg_val:
		return gpio->regs + bank->val_regs + GPIO_VAL_VALUE;
	case reg_rdata:
		return gpio->regs + bank->rdata_reg;
	case reg_dir:
		return gpio->regs + bank->val_regs + GPIO_VAL_DIR;
	case reg_irq_enable:
		return gpio->regs + bank->irq_regs + GPIO_IRQ_ENABLE;
	case reg_irq_type0:
		return gpio->regs + bank->irq_regs + GPIO_IRQ_TYPE0;
	case reg_irq_type1:
		return gpio->regs + bank->irq_regs + GPIO_IRQ_TYPE1;
	case reg_irq_type2:
		return gpio->regs + bank->irq_regs + GPIO_IRQ_TYPE2;
	case reg_irq_status:
		return gpio->regs + bank->irq_regs + GPIO_IRQ_STATUS;
	case reg_debounce_sel1:
		return gpio->regs + bank->debounce_regs + GPIO_DEBOUNCE_SEL1;
	case reg_debounce_sel2:
		return gpio->regs + bank->debounce_regs + GPIO_DEBOUNCE_SEL2;
	case reg_tolerance:
		return gpio->regs + bank->tolerance_regs;
	case reg_cmdsrc0:
		return gpio->regs + bank->cmdsrc_regs + GPIO_CMDSRC_0;
	case reg_cmdsrc1:
		return gpio->regs + bank->cmdsrc_regs + GPIO_CMDSRC_1;
	}
	BUG();
}

#define GPIO_BANK(x)	((x) >> 5)
#define GPIO_OFFSET(x)	((x) & 0x1f)
#define GPIO_BIT(x)	BIT(GPIO_OFFSET(x))

static const struct aspeed_gpio_bank *to_bank(unsigned int offset)
{
	unsigned int bank = GPIO_BANK(offset);

	WARN_ON(bank >= ARRAY_SIZE(aspeed_gpio_banks));
	return &aspeed_gpio_banks[bank];
}

static int
aspeed_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	const struct aspeed_gpio_bank *bank = to_bank(offset);
	u32 dir = readl(bank_reg(priv, bank, reg_dir));

	dir &= ~GPIO_BIT(offset);
	writel(dir, bank_reg(priv, bank, reg_dir));

	return 0;
}

static int aspeed_gpio_direction_output(struct udevice *dev, unsigned int offset,
					int value)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	const struct aspeed_gpio_bank *bank = to_bank(offset);
	u32 dir = readl(bank_reg(priv, bank, reg_dir));
	u32 output = readl(bank_reg(priv, bank, reg_rdata));

	dir |= GPIO_BIT(offset);
	writel(dir, bank_reg(priv, bank, reg_dir));

	if (value)
		output |= GPIO_BIT(offset);
	else
		output &= ~GPIO_BIT(offset);

	writel(output, bank_reg(priv, bank, reg_val));

	return 0;
}

static int aspeed_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	const struct aspeed_gpio_bank *bank = to_bank(offset);

	return !!(readl(bank_reg(priv, bank, reg_val)) & GPIO_BIT(offset));
}

static int
aspeed_gpio_set_value(struct udevice *dev, unsigned int offset, int value)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	const struct aspeed_gpio_bank *bank = to_bank(offset);
	u32 data = readl(bank_reg(priv, bank, reg_rdata));

	if (value)
		data |= GPIO_BIT(offset);
	else
		data &= ~GPIO_BIT(offset);

	writel(data, bank_reg(priv, bank, reg_val));

	return 0;
}

static int aspeed_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	const struct aspeed_gpio_bank *bank = to_bank(offset);

	if (readl(bank_reg(priv, bank, reg_dir)) & GPIO_BIT(offset))
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops aspeed_gpio_ops = {
	.direction_input	= aspeed_gpio_direction_input,
	.direction_output	= aspeed_gpio_direction_output,
	.get_value		= aspeed_gpio_get_value,
	.set_value		= aspeed_gpio_set_value,
	.get_function		= aspeed_gpio_get_function,
};

static int aspeed_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);

	uc_priv->bank_name = dev->name;
	ofnode_read_u32(dev_ofnode(dev), "ngpios", &uc_priv->gpio_count);
	priv->regs = devfdt_get_addr_ptr(dev);

	return 0;
}

static const struct udevice_id aspeed_gpio_ids[] = {
	{ .compatible = "aspeed,ast2400-gpio",  },
	{ .compatible = "aspeed,ast2500-gpio",	},
	{ .compatible = "aspeed,ast2600-gpio",	},
	{ }
};

U_BOOT_DRIVER(gpio_aspeed) = {
	.name   = "gpio-aspeed",
	.id     = UCLASS_GPIO,
	.of_match = aspeed_gpio_ids,
	.ops    = &aspeed_gpio_ops,
	.probe  = aspeed_gpio_probe,
	.priv_auto = sizeof(struct aspeed_gpio_priv),
};
