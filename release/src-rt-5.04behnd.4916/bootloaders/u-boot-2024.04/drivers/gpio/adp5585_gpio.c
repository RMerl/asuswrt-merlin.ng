// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 *
 * ADP5585 I/O Expander Controller
 *
 * Author: Alice Guo <alice.guo@nxp.com>
 */

#include <asm/gpio.h>
#include <dm.h>
#include <dt-bindings/gpio/gpio.h>
#include <i2c.h>

#define ADP5585_ID			0x00
#define ADP5585_INT_STATUS		0x01
#define ADP5585_STATUS			0x02
#define ADP5585_FIFO_1			0x03
#define ADP5585_FIFO_2			0x04
#define ADP5585_FIFO_3			0x05
#define ADP5585_FIFO_4			0x06
#define ADP5585_FIFO_5			0x07
#define ADP5585_FIFO_6			0x08
#define ADP5585_FIFO_7			0x09
#define ADP5585_FIFO_8			0x0A
#define ADP5585_FIFO_9			0x0B
#define ADP5585_FIFO_10			0x0C
#define ADP5585_FIFO_11			0x0D
#define ADP5585_FIFO_12			0x0E
#define ADP5585_FIFO_13			0x0F
#define ADP5585_FIFO_14			0x10
#define ADP5585_FIFO_15			0x11
#define ADP5585_FIFO_16			0x12
#define ADP5585_GPI_INT_STAT_A		0x13
#define ADP5585_GPI_INT_STAT_B		0x14
#define ADP5585_GPI_STATUS_A		0x15
#define ADP5585_GPI_STATUS_B		0x16
#define ADP5585_RPULL_CONFIG_A		0x17
#define ADP5585_RPULL_CONFIG_B		0x18
#define ADP5585_RPULL_CONFIG_C		0x19
#define ADP5585_RPULL_CONFIG_D		0x1A
#define ADP5585_GPI_INT_LEVEL_A		0x1B
#define ADP5585_GPI_INT_LEVEL_B		0x1C
#define ADP5585_GPI_EVENT_EN_A		0x1D
#define ADP5585_GPI_EVENT_EN_B		0x1E
#define ADP5585_GPI_INTERRUPT_EN_A	0x1F
#define ADP5585_GPI_INTERRUPT_EN_B	0x20
#define ADP5585_DEBOUNCE_DIS_A		0x21
#define ADP5585_DEBOUNCE_DIS_B		0x22
#define ADP5585_GPO_DATA_OUT_A		0x23
#define ADP5585_GPO_DATA_OUT_B		0x24
#define ADP5585_GPO_OUT_MODE_A		0x25
#define ADP5585_GPO_OUT_MODE_B		0x26
#define ADP5585_GPIO_DIRECTION_A	0x27
#define ADP5585_GPIO_DIRECTION_B	0x28
#define ADP5585_RESET1_EVENT_A		0x29
#define ADP5585_RESET1_EVENT_B		0x2A
#define ADP5585_RESET1_EVENT_C		0x2B
#define ADP5585_RESET2_EVENT_A		0x2C
#define ADP5585_RESET2_EVENT_B		0x2D
#define ADP5585_RESET_CFG		0x2E
#define ADP5585_PWM_OFFT_LOW		0x2F
#define ADP5585_PWM_OFFT_HIGH		0x30
#define ADP5585_PWM_ONT_LOW		0x31
#define ADP5585_PWM_ONT_HIGH		0x32
#define ADP5585_PWM_CFG			0x33
#define ADP5585_LOGIC_CFG		0x34
#define ADP5585_LOGIC_FF_CFG		0x35
#define ADP5585_LOGIC_INT_EVENT_EN	0x36
#define ADP5585_POLL_PTIME_CFG		0x37
#define ADP5585_PIN_CONFIG_A		0x38
#define ADP5585_PIN_CONFIG_B		0x39
#define ADP5585_PIN_CONFIG_D		0x3A
#define ADP5585_GENERAL_CFG		0x3B
#define ADP5585_INT_EN			0x3C

#define ADP5585_MAXGPIO			10
#define ADP5585_BANK(offs)		((offs) > 4)
#define ADP5585_BIT(offs)		((offs) > 4 ? \
					1u << ((offs) - 5) : 1u << (offs))

struct adp5585_plat {
	fdt_addr_t addr;
	u8 id;
	u8 dat_out[2];
	u8 dir[2];
};

static int adp5585_direction_input(struct udevice *dev, unsigned int offset)
{
	int ret;
	unsigned int bank;
	struct adp5585_plat *plat = dev_get_plat(dev);

	bank = ADP5585_BANK(offset);

	plat->dir[bank] &= ~ADP5585_BIT(offset);
	ret = dm_i2c_write(dev, ADP5585_GPIO_DIRECTION_A + bank, &plat->dir[bank], 1);

	return ret;
}

static int adp5585_direction_output(struct udevice *dev, unsigned int offset,
				    int value)
{
	int ret;
	unsigned int bank, bit;
	struct adp5585_plat *plat = dev_get_plat(dev);

	bank =  ADP5585_BANK(offset);
	bit = ADP5585_BIT(offset);

	plat->dir[bank] |= bit;

	if (value)
		plat->dat_out[bank] |= bit;
	else
		plat->dat_out[bank] &= ~bit;

	ret = dm_i2c_write(dev, ADP5585_GPO_DATA_OUT_A + bank, &plat->dat_out[bank], 1);
	ret |= dm_i2c_write(dev, ADP5585_GPIO_DIRECTION_A + bank, &plat->dir[bank], 1);

	return ret;
}

static int adp5585_get_value(struct udevice *dev, unsigned int offset)
{
	struct adp5585_plat *plat = dev_get_plat(dev);
	unsigned int bank = ADP5585_BANK(offset);
	unsigned int bit = ADP5585_BIT(offset);
	u8 val;

	if (plat->dir[bank] & bit)
		val = plat->dat_out[bank];
	else
		dm_i2c_read(dev, ADP5585_GPI_STATUS_A + bank, &val, 1);

	return !!(val & bit);
}

static int adp5585_set_value(struct udevice *dev, unsigned int offset, int value)
{
	int ret;
	unsigned int bank, bit;
	struct adp5585_plat *plat = dev_get_plat(dev);

	bank =  ADP5585_BANK(offset);
	bit = ADP5585_BIT(offset);

	if (value)
		plat->dat_out[bank] |= bit;
	else
		plat->dat_out[bank] &= ~bit;

	ret = dm_i2c_write(dev, ADP5585_GPO_DATA_OUT_A + bank, &plat->dat_out[bank], 1);

	return ret;
}

static int adp5585_get_function(struct udevice *dev, unsigned int offset)
{
	unsigned int bank, bit, dir;
	struct adp5585_plat *plat = dev_get_plat(dev);

	bank =  ADP5585_BANK(offset);
	bit = ADP5585_BIT(offset);
	dir = plat->dir[bank] & bit;

	if (!dir)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static int adp5585_xlate(struct udevice *dev, struct gpio_desc *desc,
			 struct ofnode_phandle_args *args)
{
	desc->offset =  args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops adp5585_ops = {
	.direction_input	= adp5585_direction_input,
	.direction_output	= adp5585_direction_output,
	.get_value		= adp5585_get_value,
	.set_value		= adp5585_set_value,
	.get_function		= adp5585_get_function,
	.xlate			= adp5585_xlate,
};

static int adp5585_probe(struct udevice *dev)
{
	struct adp5585_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	if (!plat)
		return 0;

	plat->addr = dev_read_addr(dev);
	if (plat->addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dm_i2c_read(dev, ADP5585_ID, &plat->id, 1);
	if (ret < 0)
		return ret;

	uc_priv->gpio_count = ADP5585_MAXGPIO;
	uc_priv->bank_name = "adp5585-gpio";

	for (int i = 0; i < 2; i++) {
		ret = dm_i2c_read(dev, ADP5585_GPO_DATA_OUT_A + i, &plat->dat_out[i], 1);
		if (ret)
			return ret;

		ret = dm_i2c_read(dev, ADP5585_GPIO_DIRECTION_A + i, &plat->dir[i], 1);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id adp5585_ids[] = {
	{ .compatible = "adp5585" },
	{ }
};

U_BOOT_DRIVER(adp5585) = {
	.name	= "adp5585",
	.id	= UCLASS_GPIO,
	.of_match	= adp5585_ids,
	.probe	= adp5585_probe,
	.ops	= &adp5585_ops,
	.plat_auto	= sizeof(struct adp5585_plat),
};
