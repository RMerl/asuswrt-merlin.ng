// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * Driver for STMicroelectronics Multi-Function eXpander (STMFX) GPIO expander
 * based on Linux driver : pinctrl/pinctrl-stmfx.c
 */
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitfield.h>
#include <power/regulator.h>

/* STMFX pins = GPIO[15:0] + aGPIO[7:0] */
#define STMFX_MAX_GPIO			16
#define STMFX_MAX_AGPIO			8

/* General */
#define STMFX_REG_CHIP_ID		0x00 /* R */
#define STMFX_REG_FW_VERSION_MSB	0x01 /* R */
#define STMFX_REG_FW_VERSION_LSB	0x02 /* R */
#define STMFX_REG_SYS_CTRL		0x40 /* RW */

/* MFX boot time is around 10ms, so after reset, we have to wait this delay */
#define STMFX_BOOT_TIME_MS 10

/* GPIOs expander */
/* GPIO_STATE1 0x10, GPIO_STATE2 0x11, GPIO_STATE3 0x12 */
#define STMFX_REG_GPIO_STATE		0x10 /* R */
/* GPIO_DIR1 0x60, GPIO_DIR2 0x61, GPIO_DIR3 0x63 */
#define STMFX_REG_GPIO_DIR		0x60 /* RW */
/* GPIO_TYPE1 0x64, GPIO_TYPE2 0x65, GPIO_TYPE3 0x66 */
#define STMFX_REG_GPIO_TYPE		0x64 /* RW */
/* GPIO_PUPD1 0x68, GPIO_PUPD2 0x69, GPIO_PUPD3 0x6A */
#define STMFX_REG_GPIO_PUPD		0x68 /* RW */
/* GPO_SET1 0x6C, GPO_SET2 0x6D, GPO_SET3 0x6E */
#define STMFX_REG_GPO_SET		0x6C /* RW */
/* GPO_CLR1 0x70, GPO_CLR2 0x71, GPO_CLR3 0x72 */
#define STMFX_REG_GPO_CLR		0x70 /* RW */

/* STMFX_REG_CHIP_ID bitfields */
#define STMFX_REG_CHIP_ID_MASK		GENMASK(7, 0)

/* STMFX_REG_SYS_CTRL bitfields */
#define STMFX_REG_SYS_CTRL_GPIO_EN	BIT(0)
#define STMFX_REG_SYS_CTRL_ALTGPIO_EN	BIT(3)
#define STMFX_REG_SYS_CTRL_SWRST	BIT(7)

#define NR_GPIO_REGS			3
#define NR_GPIOS_PER_REG		8
#define get_reg(offset)			((offset) / NR_GPIOS_PER_REG)
#define get_shift(offset)		((offset) % NR_GPIOS_PER_REG)
#define get_mask(offset)		(BIT(get_shift(offset)))

struct stmfx_pinctrl {
	struct udevice *gpio;
};

static int stmfx_read(struct udevice *dev, uint offset)
{
	return  dm_i2c_reg_read(dev_get_parent(dev), offset);
}

static int stmfx_write(struct udevice *dev, uint offset, unsigned int val)
{
	return dm_i2c_reg_write(dev_get_parent(dev), offset, val);
}

static int stmfx_gpio_get(struct udevice *dev, unsigned int offset)
{
	u32 reg = STMFX_REG_GPIO_STATE + get_reg(offset);
	u32 mask = get_mask(offset);
	int ret;

	ret = stmfx_read(dev, reg);

	return ret < 0 ? ret : !!(ret & mask);
}

static int stmfx_gpio_set(struct udevice *dev, unsigned int offset, int value)
{
	u32 reg = value ? STMFX_REG_GPO_SET : STMFX_REG_GPO_CLR;
	u32 mask = get_mask(offset);

	return stmfx_write(dev, reg + get_reg(offset), mask);
}

static int stmfx_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	u32 reg = STMFX_REG_GPIO_DIR + get_reg(offset);
	u32 mask = get_mask(offset);
	int ret;

	ret = stmfx_read(dev, reg);

	if (ret < 0)
		return ret;
	/* On stmfx, gpio pins direction is (0)input, (1)output. */

	return ret & mask ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int stmfx_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	u32 reg = STMFX_REG_GPIO_DIR + get_reg(offset);
	u32 mask = get_mask(offset);
	int ret;

	ret = stmfx_read(dev, reg);
	if (ret < 0)
		return ret;

	ret &= ~mask;

	return stmfx_write(dev, reg, ret & ~mask);
}

static int stmfx_gpio_direction_output(struct udevice *dev,
				       unsigned int offset, int value)
{
	u32 reg = STMFX_REG_GPIO_DIR + get_reg(offset);
	u32 mask = get_mask(offset);
	int ret;

	ret = stmfx_gpio_set(dev, offset, value);
	if (ret < 0)
		return ret;

	ret = stmfx_read(dev, reg);
	if (ret < 0)
		return ret;

	return stmfx_write(dev, reg, ret | mask);
}

static int stmfx_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	u8 sys_ctrl;

	uc_priv->bank_name = "stmfx";
	uc_priv->gpio_count = STMFX_MAX_GPIO + STMFX_MAX_AGPIO;
	if (!dev_read_phandle_with_args(dev, "gpio-ranges",
					NULL, 3, 0, &args)) {
		uc_priv->gpio_count = args.args[2];
	}

	/* enable GPIO function */
	sys_ctrl = STMFX_REG_SYS_CTRL_GPIO_EN;
	if (uc_priv->gpio_count > STMFX_MAX_GPIO)
		sys_ctrl |= STMFX_REG_SYS_CTRL_ALTGPIO_EN;
	stmfx_write(dev, STMFX_REG_SYS_CTRL, sys_ctrl);

	return 0;
}

static const struct dm_gpio_ops stmfx_gpio_ops = {
	.set_value = stmfx_gpio_set,
	.get_value = stmfx_gpio_get,
	.get_function = stmfx_gpio_get_function,
	.direction_input = stmfx_gpio_direction_input,
	.direction_output = stmfx_gpio_direction_output,
};

U_BOOT_DRIVER(stmfx_gpio) = {
	.name	= "stmfx-gpio",
	.id	= UCLASS_GPIO,
	.probe	= stmfx_gpio_probe,
	.ops	= &stmfx_gpio_ops,
};

#if CONFIG_IS_ENABLED(PINCONF)
static const struct pinconf_param stmfx_pinctrl_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 0 },
	{ "bias-pull-pin-default", PIN_CONFIG_BIAS_PULL_PIN_DEFAULT, 0 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 0 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 0 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1 },
	{ "output-low", PIN_CONFIG_OUTPUT, 0 },
};

static int stmfx_pinctrl_set_pupd(struct udevice *dev,
				  unsigned int pin, u32 pupd)
{
	u8 reg = STMFX_REG_GPIO_PUPD + get_reg(pin);
	u32 mask = get_mask(pin);
	int ret;

	ret = stmfx_read(dev, reg);
	if (ret < 0)
		return ret;
	ret = (ret & ~mask) | (pupd ? mask : 0);

	return stmfx_write(dev, reg, ret);
}

static int stmfx_pinctrl_set_type(struct udevice *dev,
				  unsigned int pin, u32 type)
{
	u8 reg = STMFX_REG_GPIO_TYPE + get_reg(pin);
	u32 mask = get_mask(pin);
	int ret;

	ret = stmfx_read(dev, reg);
	if (ret < 0)
		return ret;
	ret = (ret & ~mask) | (type ? mask : 0);

	return stmfx_write(dev, reg, ret);
}

static int stmfx_pinctrl_conf_set(struct udevice *dev, unsigned int pin,
				  unsigned int param, unsigned int arg)
{
	int ret, dir;
	struct stmfx_pinctrl *plat = dev_get_platdata(dev);

	dir = stmfx_gpio_get_function(plat->gpio, pin);

	if (dir < 0)
		return dir;

	switch (param) {
	case PIN_CONFIG_BIAS_PULL_PIN_DEFAULT:
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
		ret = stmfx_pinctrl_set_pupd(dev, pin, 0);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		ret = stmfx_pinctrl_set_pupd(dev, pin, 1);
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		if (dir == GPIOF_OUTPUT)
			ret = stmfx_pinctrl_set_type(dev, pin, 1);
		else
			ret = stmfx_pinctrl_set_type(dev, pin, 0);
		break;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		if (dir == GPIOF_OUTPUT)
			ret = stmfx_pinctrl_set_type(dev, pin, 0);
		else
			ret = stmfx_pinctrl_set_type(dev, pin, 1);
		break;
	case PIN_CONFIG_OUTPUT:
		ret = stmfx_gpio_direction_output(plat->gpio, pin, arg);
		break;
	default:
		return -ENOTSUPP;
	}

	return ret;
}
#endif

static int stmfx_pinctrl_get_pins_count(struct udevice *dev)
{
	struct stmfx_pinctrl *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(plat->gpio);

	return uc_priv->gpio_count;
}

/*
 * STMFX pins[15:0] are called "gpio[15:0]"
 * and STMFX pins[23:16] are called "agpio[7:0]"
 */
#define MAX_PIN_NAME_LEN 7
static char pin_name[MAX_PIN_NAME_LEN];
static const char *stmfx_pinctrl_get_pin_name(struct udevice *dev,
					      unsigned int selector)
{
	if (selector < STMFX_MAX_GPIO)
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "agpio%u", selector - 16);
	return pin_name;
}

static int stmfx_pinctrl_get_pin_muxing(struct udevice *dev,
					unsigned int selector,
					char *buf, int size)
{
	struct stmfx_pinctrl *plat = dev_get_platdata(dev);
	int func;

	func = stmfx_gpio_get_function(plat->gpio, selector);
	if (func < 0)
		return func;

	snprintf(buf, size, "%s", func == GPIOF_INPUT ? "input" : "output");

	return 0;
}

static int stmfx_pinctrl_bind(struct udevice *dev)
{
	struct stmfx_pinctrl *plat = dev_get_platdata(dev);

	return device_bind_driver_to_node(dev->parent,
					  "stmfx-gpio", "stmfx-gpio",
					  dev_ofnode(dev), &plat->gpio);
};

static int stmfx_pinctrl_probe(struct udevice *dev)
{
	struct stmfx_pinctrl *plat = dev_get_platdata(dev);

	return device_probe(plat->gpio);
};

const struct pinctrl_ops stmfx_pinctrl_ops = {
	.get_pins_count = stmfx_pinctrl_get_pins_count,
	.get_pin_name = stmfx_pinctrl_get_pin_name,
	.set_state = pinctrl_generic_set_state,
	.get_pin_muxing	= stmfx_pinctrl_get_pin_muxing,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_set = stmfx_pinctrl_conf_set,
	.pinconf_num_params = ARRAY_SIZE(stmfx_pinctrl_conf_params),
	.pinconf_params = stmfx_pinctrl_conf_params,
#endif
};

static const struct udevice_id stmfx_pinctrl_match[] = {
	{ .compatible = "st,stmfx-0300-pinctrl", },
};

U_BOOT_DRIVER(stmfx_pinctrl) = {
	.name = "stmfx-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(stmfx_pinctrl_match),
	.bind = stmfx_pinctrl_bind,
	.probe = stmfx_pinctrl_probe,
	.ops = &stmfx_pinctrl_ops,
	.platdata_auto_alloc_size = sizeof(struct stmfx_pinctrl),
};

static int stmfx_chip_init(struct udevice *dev)
{
	u8 id;
	u8 version[2];
	int ret;
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);

	id = dm_i2c_reg_read(dev, STMFX_REG_CHIP_ID);
	if (id < 0) {
		dev_err(dev, "error reading chip id: %d\n", id);
		return ret;
	}
	/*
	 * Check that ID is the complement of the I2C address:
	 * STMFX I2C address follows the 7-bit format (MSB), that's why
	 * client->addr is shifted.
	 *
	 * STMFX_I2C_ADDR|       STMFX         |        Linux
	 *   input pin   | I2C device address  | I2C device address
	 *---------------------------------------------------------
	 *       0       | b: 1000 010x h:0x84 |       0x42
	 *       1       | b: 1000 011x h:0x86 |       0x43
	 */
	if (FIELD_GET(STMFX_REG_CHIP_ID_MASK, ~id) != (chip->chip_addr << 1)) {
		dev_err(dev, "unknown chip id: %#x\n", id);
		return -EINVAL;
	}

	ret = dm_i2c_read(dev, STMFX_REG_FW_VERSION_MSB,
			  version, sizeof(version));
	if (ret) {
		dev_err(dev, "error reading fw version: %d\n", ret);
		return ret;
	}

	dev_info(dev, "STMFX id: %#x, fw version: %x.%02x\n",
		 id, version[0], version[1]);

	ret = dm_i2c_reg_read(dev, STMFX_REG_SYS_CTRL);

	if (ret < 0)
		return ret;

	ret = dm_i2c_reg_write(dev, STMFX_REG_SYS_CTRL,
			       ret | STMFX_REG_SYS_CTRL_SWRST);
	if (ret)
		return ret;

	mdelay(STMFX_BOOT_TIME_MS);

	return ret;
}

static int stmfx_probe(struct udevice *dev)
{
	struct udevice *vdd;
	int ret;

	ret = device_get_supply_regulator(dev, "vdd-supply", &vdd);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "vdd regulator error:%d\n", ret);
		return ret;
	}
	if (!ret) {
		ret = regulator_set_enable(vdd, true);
		if (ret) {
			dev_err(dev, "vdd enable failed: %d\n", ret);
			return ret;
		}
	}

	return stmfx_chip_init(dev);
}

static const struct udevice_id stmfx_match[] = {
	{ .compatible = "st,stmfx-0300", },
};

U_BOOT_DRIVER(stmfx) = {
	.name = "stmfx",
	.id = UCLASS_I2C_GENERIC,
	.of_match = of_match_ptr(stmfx_match),
	.probe = stmfx_probe,
	.bind = dm_scan_fdt_dev,
};
