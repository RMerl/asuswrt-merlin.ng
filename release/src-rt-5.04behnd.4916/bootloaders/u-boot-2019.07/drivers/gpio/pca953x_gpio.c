// SPDX-License-Identifier: GPL-2.0+
/*
 * Take linux kernel driver drivers/gpio/gpio-pca953x.c for reference.
 *
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 *
 */

/*
 * Note:
 * The driver's compatible table is borrowed from Linux Kernel,
 * but now max supported gpio pins is 24 and only PCA953X_TYPE
 * is supported. PCA957X_TYPE is not supported now.
 * Also the Polarity Inversion feature is not supported now.
 *
 * TODO:
 * 1. Support PCA957X_TYPE
 * 2. Support 24 gpio pins
 * 3. Support Polarity Inversion
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dt-bindings/gpio/gpio.h>

#define PCA953X_INPUT           0
#define PCA953X_OUTPUT          1
#define PCA953X_INVERT          2
#define PCA953X_DIRECTION       3

#define PCA_GPIO_MASK           0x00FF
#define PCA_INT                 0x0100
#define PCA953X_TYPE            0x1000
#define PCA957X_TYPE            0x2000
#define PCA_TYPE_MASK           0xF000
#define PCA_CHIP_TYPE(x)        ((x) & PCA_TYPE_MASK)

enum {
	PCA953X_DIRECTION_IN,
	PCA953X_DIRECTION_OUT,
};

#define MAX_BANK 5
#define BANK_SZ 8

/*
 * struct pca953x_info - Data for pca953x
 *
 * @dev: udevice structure for the device
 * @addr: i2c slave address
 * @invert: Polarity inversion or not
 * @gpio_count: the number of gpio pins that the device supports
 * @chip_type: indicate the chip type,PCA953X or PCA957X
 * @bank_count: the number of banks that the device supports
 * @reg_output: array to hold the value of output registers
 * @reg_direction: array to hold the value of direction registers
 */
struct pca953x_info {
	struct udevice *dev;
	int addr;
	int invert;
	int gpio_count;
	int chip_type;
	int bank_count;
	u8 reg_output[MAX_BANK];
	u8 reg_direction[MAX_BANK];
};

static int pca953x_write_single(struct udevice *dev, int reg, u8 val,
				int offset)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int bank_shift = fls((info->gpio_count - 1) / BANK_SZ);
	int off = offset / BANK_SZ;
	int ret = 0;

	ret = dm_i2c_write(dev, (reg << bank_shift) + off, &val, 1);
	if (ret) {
		dev_err(dev, "%s error\n", __func__);
		return ret;
	}

	return 0;
}

static int pca953x_read_single(struct udevice *dev, int reg, u8 *val,
			       int offset)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int bank_shift = fls((info->gpio_count - 1) / BANK_SZ);
	int off = offset / BANK_SZ;
	int ret;
	u8 byte;

	ret = dm_i2c_read(dev, (reg << bank_shift) + off, &byte, 1);
	if (ret) {
		dev_err(dev, "%s error\n", __func__);
		return ret;
	}

	*val = byte;

	return 0;
}

static int pca953x_read_regs(struct udevice *dev, int reg, u8 *val)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int ret = 0;

	if (info->gpio_count <= 8) {
		ret = dm_i2c_read(dev, reg, val, 1);
	} else if (info->gpio_count <= 16) {
		ret = dm_i2c_read(dev, reg << 1, val, info->bank_count);
	} else if (info->gpio_count == 40) {
		/* Auto increment */
		ret = dm_i2c_read(dev, (reg << 3) | 0x80, val,
				  info->bank_count);
	} else {
		dev_err(dev, "Unsupported now\n");
		return -EINVAL;
	}

	return ret;
}

static int pca953x_write_regs(struct udevice *dev, int reg, u8 *val)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int ret = 0;

	if (info->gpio_count <= 8) {
		ret = dm_i2c_write(dev, reg, val, 1);
	} else if (info->gpio_count <= 16) {
		ret = dm_i2c_write(dev, reg << 1, val, info->bank_count);
	} else if (info->gpio_count == 40) {
		/* Auto increment */
		ret = dm_i2c_write(dev, (reg << 3) | 0x80, val, info->bank_count);
	} else {
		return -EINVAL;
	}

	return ret;
}

static int pca953x_is_output(struct udevice *dev, int offset)
{
	struct pca953x_info *info = dev_get_platdata(dev);

	int bank = offset / BANK_SZ;
	int off = offset % BANK_SZ;

	/*0: output; 1: input */
	return !(info->reg_direction[bank] & (1 << off));
}

static int pca953x_get_value(struct udevice *dev, uint offset)
{
	int ret;
	u8 val = 0;

	int off = offset % BANK_SZ;

	ret = pca953x_read_single(dev, PCA953X_INPUT, &val, offset);
	if (ret)
		return ret;

	return (val >> off) & 0x1;
}

static int pca953x_set_value(struct udevice *dev, uint offset, int value)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int bank = offset / BANK_SZ;
	int off = offset % BANK_SZ;
	u8 val;
	int ret;

	if (value)
		val = info->reg_output[bank] | (1 << off);
	else
		val = info->reg_output[bank] & ~(1 << off);

	ret = pca953x_write_single(dev, PCA953X_OUTPUT, val, offset);
	if (ret)
		return ret;

	info->reg_output[bank] = val;

	return 0;
}

static int pca953x_set_direction(struct udevice *dev, uint offset, int dir)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	int bank = offset / BANK_SZ;
	int off = offset % BANK_SZ;
	u8 val;
	int ret;

	if (dir == PCA953X_DIRECTION_IN)
		val = info->reg_direction[bank] | (1 << off);
	else
		val = info->reg_direction[bank] & ~(1 << off);

	ret = pca953x_write_single(dev, PCA953X_DIRECTION, val, offset);
	if (ret)
		return ret;

	info->reg_direction[bank] = val;

	return 0;
}

static int pca953x_direction_input(struct udevice *dev, uint offset)
{
	return pca953x_set_direction(dev, offset, PCA953X_DIRECTION_IN);
}

static int pca953x_direction_output(struct udevice *dev, uint offset, int value)
{
	/* Configure output value. */
	pca953x_set_value(dev, offset, value);

	/* Configure direction as output. */
	pca953x_set_direction(dev, offset, PCA953X_DIRECTION_OUT);

	return 0;
}

static int pca953x_get_function(struct udevice *dev, uint offset)
{
	if (pca953x_is_output(dev, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int pca953x_xlate(struct udevice *dev, struct gpio_desc *desc,
			 struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static const struct dm_gpio_ops pca953x_ops = {
	.direction_input	= pca953x_direction_input,
	.direction_output	= pca953x_direction_output,
	.get_value		= pca953x_get_value,
	.set_value		= pca953x_set_value,
	.get_function		= pca953x_get_function,
	.xlate			= pca953x_xlate,
};

static int pca953x_probe(struct udevice *dev)
{
	struct pca953x_info *info = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[32], label[8], *str;
	int addr;
	ulong driver_data;
	int ret;
	int size;
	const u8 *tmp;
	u8 val[MAX_BANK];

	addr = dev_read_addr(dev);
	if (addr == 0)
		return -ENODEV;

	info->addr = addr;

	driver_data = dev_get_driver_data(dev);

	info->gpio_count = driver_data & PCA_GPIO_MASK;
	if (info->gpio_count > MAX_BANK * BANK_SZ) {
		dev_err(dev, "Max support %d pins now\n", MAX_BANK * BANK_SZ);
		return -EINVAL;
	}

	info->chip_type = PCA_CHIP_TYPE(driver_data);
	if (info->chip_type != PCA953X_TYPE) {
		dev_err(dev, "Only support PCA953X chip type now.\n");
		return -EINVAL;
	}

	info->bank_count = DIV_ROUND_UP(info->gpio_count, BANK_SZ);

	ret = pca953x_read_regs(dev, PCA953X_OUTPUT, info->reg_output);
	if (ret) {
		dev_err(dev, "Error reading output register\n");
		return ret;
	}

	ret = pca953x_read_regs(dev, PCA953X_DIRECTION, info->reg_direction);
	if (ret) {
		dev_err(dev, "Error reading direction register\n");
		return ret;
	}

	tmp = dev_read_prop(dev, "label", &size);

	if (tmp) {
		memcpy(label, tmp, sizeof(label) - 1);
		label[sizeof(label) - 1] = '\0';
		snprintf(name, sizeof(name), "%s@%x_", label, info->addr);
	} else {
		snprintf(name, sizeof(name), "gpio@%x_", info->addr);
	}

	/* Clear the polarity registers to no invert */
	memset(val, 0, MAX_BANK);
	ret = pca953x_write_regs(dev, PCA953X_INVERT, val);
	if (ret < 0) {
		dev_err(dev, "Error writing invert register\n");
		return ret;
	}

	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = info->gpio_count;

	dev_dbg(dev, "%s is ready\n", str);

	return 0;
}

#define OF_953X(__nrgpio, __int) (ulong)(__nrgpio | PCA953X_TYPE | __int)
#define OF_957X(__nrgpio, __int) (ulong)(__nrgpio | PCA957X_TYPE | __int)

static const struct udevice_id pca953x_ids[] = {
	{ .compatible = "nxp,pca9505", .data = OF_953X(40, PCA_INT), },
	{ .compatible = "nxp,pca9534", .data = OF_953X(8, PCA_INT), },
	{ .compatible = "nxp,pca9535", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "nxp,pca9536", .data = OF_953X(4, 0), },
	{ .compatible = "nxp,pca9537", .data = OF_953X(4, PCA_INT), },
	{ .compatible = "nxp,pca9538", .data = OF_953X(8, PCA_INT), },
	{ .compatible = "nxp,pca9539", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "nxp,pca9554", .data = OF_953X(8, PCA_INT), },
	{ .compatible = "nxp,pca9555", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "nxp,pca9556", .data = OF_953X(8, 0), },
	{ .compatible = "nxp,pca9557", .data = OF_953X(8, 0), },
	{ .compatible = "nxp,pca9574", .data = OF_957X(8, PCA_INT), },
	{ .compatible = "nxp,pca9575", .data = OF_957X(16, PCA_INT), },
	{ .compatible = "nxp,pca9698", .data = OF_953X(40, 0), },

	{ .compatible = "maxim,max7310", .data = OF_953X(8, 0), },
	{ .compatible = "maxim,max7312", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "maxim,max7313", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "maxim,max7315", .data = OF_953X(8, PCA_INT), },

	{ .compatible = "ti,pca6107", .data = OF_953X(8, PCA_INT), },
	{ .compatible = "ti,tca6408", .data = OF_953X(8, PCA_INT), },
	{ .compatible = "ti,tca6416", .data = OF_953X(16, PCA_INT), },
	{ .compatible = "ti,tca6424", .data = OF_953X(24, PCA_INT), },

	{ .compatible = "onsemi,pca9654", .data = OF_953X(8, PCA_INT), },

	{ .compatible = "exar,xra1202", .data = OF_953X(8, 0), },
	{ }
};

U_BOOT_DRIVER(pca953x) = {
	.name		= "pca953x",
	.id		= UCLASS_GPIO,
	.ops		= &pca953x_ops,
	.probe		= pca953x_probe,
	.platdata_auto_alloc_size = sizeof(struct pca953x_info),
	.of_match	= pca953x_ids,
};
