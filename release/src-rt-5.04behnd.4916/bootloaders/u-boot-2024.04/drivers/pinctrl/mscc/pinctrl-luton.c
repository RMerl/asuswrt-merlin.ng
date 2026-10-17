// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <gregory.clement@bootlin.com>
 * License: Dual MIT/GPL
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include "mscc-common.h"

enum {
	FUNC_NONE,
	FUNC_GPIO,
	FUNC_SIO,
	FUNC_TACHO,
	FUNC_TWI,
	FUNC_PHY_LED,
	FUNC_EXT_IRQ,
	FUNC_SFP,
	FUNC_SI,
	FUNC_PWM,
	FUNC_UART,
	FUNC_MAX
};

static char * const luton_function_names[] = {
	[FUNC_NONE]		= "none",
	[FUNC_GPIO]		= "gpio",
	[FUNC_SIO]		= "sio",
	[FUNC_TACHO]		= "tacho",
	[FUNC_TWI]		= "twi",
	[FUNC_PHY_LED]		= "phy_led",
	[FUNC_EXT_IRQ]		= "ext_irq",
	[FUNC_SFP]		= "sfp",
	[FUNC_SI]		= "si",
	[FUNC_PWM]		= "pwm",
	[FUNC_UART]		= "uart",
};

MSCC_P(0,  SIO,       NONE,      NONE);
MSCC_P(1,  SIO,       NONE,      NONE);
MSCC_P(2,  SIO,       NONE,      NONE);
MSCC_P(3,  SIO,       NONE,      NONE);
MSCC_P(4,  TACHO,     NONE,      NONE);
MSCC_P(5,  TWI,       PHY_LED,   NONE);
MSCC_P(6,  TWI,       PHY_LED,   NONE);
MSCC_P(7,  NONE,      PHY_LED,   NONE);
MSCC_P(8,  EXT_IRQ,   PHY_LED,   NONE);
MSCC_P(9,  EXT_IRQ,   PHY_LED,   NONE);
MSCC_P(10, SFP,       PHY_LED,   NONE);
MSCC_P(11, SFP,       PHY_LED,   NONE);
MSCC_P(12, SFP,       PHY_LED,   NONE);
MSCC_P(13, SFP,       PHY_LED,   NONE);
MSCC_P(14, SI,        PHY_LED,   NONE);
MSCC_P(15, SI,        PHY_LED,   NONE);
MSCC_P(16, SI,        PHY_LED,   NONE);
MSCC_P(17, SFP,       PHY_LED,   NONE);
MSCC_P(18, SFP,       PHY_LED,   NONE);
MSCC_P(19, SFP,       PHY_LED,   NONE);
MSCC_P(20, SFP,       PHY_LED,   NONE);
MSCC_P(21, SFP,       PHY_LED,   NONE);
MSCC_P(22, SFP,       PHY_LED,   NONE);
MSCC_P(23, SFP,       PHY_LED,   NONE);
MSCC_P(24, SFP,       PHY_LED,   NONE);
MSCC_P(25, SFP,       PHY_LED,   NONE);
MSCC_P(26, SFP,       PHY_LED,   NONE);
MSCC_P(27, SFP,       PHY_LED,   NONE);
MSCC_P(28, SFP,       PHY_LED,   NONE);
MSCC_P(29, PWM,       NONE,      NONE);
MSCC_P(30, UART,      NONE,      NONE);
MSCC_P(31, UART,      NONE,      NONE);

#define LUTON_PIN(n) {						\
	.name = "GPIO_"#n,					\
	.drv_data = &mscc_pin_##n				\
}

static const struct mscc_pin_data luton_pins[] = {
	LUTON_PIN(0),
	LUTON_PIN(1),
	LUTON_PIN(2),
	LUTON_PIN(3),
	LUTON_PIN(4),
	LUTON_PIN(5),
	LUTON_PIN(6),
	LUTON_PIN(7),
	LUTON_PIN(8),
	LUTON_PIN(9),
	LUTON_PIN(10),
	LUTON_PIN(11),
	LUTON_PIN(12),
	LUTON_PIN(13),
	LUTON_PIN(14),
	LUTON_PIN(15),
	LUTON_PIN(16),
	LUTON_PIN(17),
	LUTON_PIN(18),
	LUTON_PIN(19),
	LUTON_PIN(20),
	LUTON_PIN(21),
	LUTON_PIN(22),
	LUTON_PIN(23),
	LUTON_PIN(24),
	LUTON_PIN(25),
	LUTON_PIN(26),
	LUTON_PIN(27),
	LUTON_PIN(28),
	LUTON_PIN(29),
	LUTON_PIN(30),
	LUTON_PIN(31),
};

static const unsigned long luton_gpios[] = {
	[MSCC_GPIO_OUT_SET] = 0x00,
	[MSCC_GPIO_OUT_CLR] = 0x04,
	[MSCC_GPIO_OUT] = 0x08,
	[MSCC_GPIO_IN] = 0x0c,
	[MSCC_GPIO_OE] = 0x10,
	[MSCC_GPIO_INTR] = 0x14,
	[MSCC_GPIO_INTR_ENA] = 0x18,
	[MSCC_GPIO_INTR_IDENT] = 0x1c,
	[MSCC_GPIO_ALT0] = 0x20,
	[MSCC_GPIO_ALT1] = 0x24,
};

static int luton_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "luton-gpio";
	uc_priv->gpio_count = ARRAY_SIZE(luton_pins);

	return 0;
}

static struct driver luton_gpio_driver = {
	.name	= "luton-gpio",
	.id	= UCLASS_GPIO,
	.probe	= luton_gpio_probe,
	.ops	= &mscc_gpio_ops,
};

int luton_pinctrl_probe(struct udevice *dev)
{
	int ret;

	ret = mscc_pinctrl_probe(dev, FUNC_MAX, luton_pins,
				 ARRAY_SIZE(luton_pins), luton_function_names,
				 luton_gpios);

	if (ret)
		return ret;

	ret = device_bind(dev, &luton_gpio_driver, "luton-gpio", NULL,
			  dev_ofnode(dev), NULL);

	return 0;
}

static const struct udevice_id luton_pinctrl_of_match[] = {
	{.compatible = "mscc,luton-pinctrl"},
	{},
};

U_BOOT_DRIVER(luton_pinctrl) = {
	.name = "luton-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(luton_pinctrl_of_match),
	.probe = luton_pinctrl_probe,
	.priv_auto	= sizeof(struct mscc_pinctrl),
	.ops = &mscc_pinctrl_ops,
};
