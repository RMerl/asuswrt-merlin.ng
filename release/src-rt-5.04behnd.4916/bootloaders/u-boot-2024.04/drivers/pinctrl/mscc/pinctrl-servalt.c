// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <horatiu.vultur@microchip.com>
 * Copyright (c) 2019 Microsemi Corporation
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
	FUNC_IRQ0_IN,
	FUNC_IRQ0_OUT,
	FUNC_IRQ1_IN,
	FUNC_IRQ1_OUT,
	FUNC_MIIM1,
	FUNC_MIIM2,
	FUNC_PCI_WAKE,
	FUNC_PTP0,
	FUNC_PTP1,
	FUNC_PTP2,
	FUNC_PTP3,
	FUNC_PWM,
	FUNC_RCVRD_CLK0,
	FUNC_RCVRD_CLK1,
	FUNC_RCVRD_CLK2,
	FUNC_RCVRD_CLK3,
	FUNC_REF_CLK0,
	FUNC_REF_CLK1,
	FUNC_REF_CLK2,
	FUNC_REF_CLK3,
	FUNC_SFP0,
	FUNC_SFP1,
	FUNC_SFP2,
	FUNC_SFP3,
	FUNC_SFP4,
	FUNC_SFP5,
	FUNC_SFP6,
	FUNC_SFP7,
	FUNC_SFP8,
	FUNC_SFP9,
	FUNC_SFP10,
	FUNC_SFP11,
	FUNC_SFP12,
	FUNC_SFP13,
	FUNC_SFP14,
	FUNC_SFP15,
	FUNC_SIO,
	FUNC_SPI,
	FUNC_TACHO,
	FUNC_TWI,
	FUNC_TWI2,
	FUNC_TWI_SCL_M,
	FUNC_UART,
	FUNC_UART2,
	FUNC_MAX
};

static char * const servalt_function_names[] = {
	[FUNC_NONE]		= "none",
	[FUNC_GPIO]		= "gpio",
	[FUNC_IRQ0_IN]		= "irq0_in",
	[FUNC_IRQ0_OUT]		= "irq0_out",
	[FUNC_IRQ1_IN]		= "irq1_in",
	[FUNC_IRQ1_OUT]		= "irq1_out",
	[FUNC_MIIM1]		= "miim1",
	[FUNC_MIIM2]		= "miim2",
	[FUNC_PCI_WAKE]		= "pci_wake",
	[FUNC_PTP0]		= "ptp0",
	[FUNC_PTP1]		= "ptp1",
	[FUNC_PTP2]		= "ptp2",
	[FUNC_PTP3]		= "ptp3",
	[FUNC_PWM]		= "pwm",
	[FUNC_RCVRD_CLK0]	= "rcvrd_clk0",
	[FUNC_RCVRD_CLK1]	= "rcvrd_clk1",
	[FUNC_RCVRD_CLK2]	= "rcvrd_clk2",
	[FUNC_RCVRD_CLK3]	= "rcvrd_clk3",
	[FUNC_REF_CLK0]		= "ref_clk0",
	[FUNC_REF_CLK1]		= "ref_clk1",
	[FUNC_REF_CLK2]		= "ref_clk2",
	[FUNC_REF_CLK3]		= "ref_clk3",
	[FUNC_SFP0]		= "sfp0",
	[FUNC_SFP1]		= "sfp1",
	[FUNC_SFP2]		= "sfp2",
	[FUNC_SFP3]		= "sfp3",
	[FUNC_SFP4]		= "sfp4",
	[FUNC_SFP5]		= "sfp5",
	[FUNC_SFP6]		= "sfp6",
	[FUNC_SFP7]		= "sfp7",
	[FUNC_SFP8]		= "sfp8",
	[FUNC_SFP9]		= "sfp9",
	[FUNC_SFP10]		= "sfp10",
	[FUNC_SFP11]		= "sfp11",
	[FUNC_SFP12]		= "sfp12",
	[FUNC_SFP13]		= "sfp13",
	[FUNC_SFP14]		= "sfp14",
	[FUNC_SFP15]		= "sfp15",
	[FUNC_SIO]		= "sio",
	[FUNC_SPI]		= "spi",
	[FUNC_TACHO]		= "tacho",
	[FUNC_TWI]		= "twi",
	[FUNC_TWI2]		= "twi2",
	[FUNC_TWI_SCL_M]	= "twi_scl_m",
	[FUNC_UART]		= "uart",
	[FUNC_UART2]		= "uart2",
};

MSCC_P(0,  SIO,        NONE,      NONE);
MSCC_P(1,  SIO,        NONE,      NONE);
MSCC_P(2,  SIO,        NONE,      NONE);
MSCC_P(3,  SIO,        NONE,      NONE);
MSCC_P(4,  IRQ0_IN,    IRQ0_OUT,  TWI_SCL_M);
MSCC_P(5,  IRQ1_IN,    IRQ1_OUT,  TWI_SCL_M);
MSCC_P(6,  UART,       NONE,      NONE);
MSCC_P(7,  UART,       NONE,      NONE);
MSCC_P(8,  SPI,        SFP0,      TWI_SCL_M);
MSCC_P(9,  PCI_WAKE,   SFP1,      SPI);
MSCC_P(10, PTP0,       SFP2,      TWI_SCL_M);
MSCC_P(11, PTP1,       SFP3,      TWI_SCL_M);
MSCC_P(12, REF_CLK0,   SFP4,      TWI_SCL_M);
MSCC_P(13, REF_CLK1,   SFP5,      TWI_SCL_M);
MSCC_P(14, REF_CLK2,   IRQ0_OUT,  SPI);
MSCC_P(15, REF_CLK3,   IRQ1_OUT,  SPI);
MSCC_P(16, TACHO,      SFP6,      SPI);
MSCC_P(17, PWM,        NONE,      TWI_SCL_M);
MSCC_P(18, PTP2,       SFP7,      SPI);
MSCC_P(19, PTP3,       SFP8,      SPI);
MSCC_P(20, UART2,      SFP9,      SPI);
MSCC_P(21, UART2,      NONE,      NONE);
MSCC_P(22, MIIM1,      SFP10,     TWI2);
MSCC_P(23, MIIM1,      SFP11,     TWI2);
MSCC_P(24, TWI,        NONE,      NONE);
MSCC_P(25, TWI,        SFP12,     TWI_SCL_M);
MSCC_P(26, TWI_SCL_M,  SFP13,     SPI);
MSCC_P(27, TWI_SCL_M,  SFP14,     SPI);
MSCC_P(28, TWI_SCL_M,  SFP15,     SPI);
MSCC_P(29, TWI_SCL_M,  NONE,      NONE);
MSCC_P(30, TWI_SCL_M,  NONE,      NONE);
MSCC_P(31, TWI_SCL_M,  NONE,      NONE);
MSCC_P(32, TWI_SCL_M,  NONE,      NONE);
MSCC_P(33, RCVRD_CLK0, NONE,      NONE);
MSCC_P(34, RCVRD_CLK1, NONE,      NONE);
MSCC_P(35, RCVRD_CLK2, NONE,      NONE);
MSCC_P(36, RCVRD_CLK3, NONE,      NONE);

#define SERVALT_PIN(n) {					\
	.name = "GPIO_"#n,					\
	.drv_data = &mscc_pin_##n				\
}

static const struct mscc_pin_data servalt_pins[] = {
	SERVALT_PIN(0),
	SERVALT_PIN(1),
	SERVALT_PIN(2),
	SERVALT_PIN(3),
	SERVALT_PIN(4),
	SERVALT_PIN(5),
	SERVALT_PIN(6),
	SERVALT_PIN(7),
	SERVALT_PIN(8),
	SERVALT_PIN(9),
	SERVALT_PIN(10),
	SERVALT_PIN(11),
	SERVALT_PIN(12),
	SERVALT_PIN(13),
	SERVALT_PIN(14),
	SERVALT_PIN(15),
	SERVALT_PIN(16),
	SERVALT_PIN(17),
	SERVALT_PIN(18),
	SERVALT_PIN(19),
	SERVALT_PIN(20),
	SERVALT_PIN(21),
	SERVALT_PIN(22),
	SERVALT_PIN(23),
	SERVALT_PIN(24),
	SERVALT_PIN(25),
	SERVALT_PIN(26),
	SERVALT_PIN(27),
	SERVALT_PIN(28),
	SERVALT_PIN(29),
	SERVALT_PIN(30),
	SERVALT_PIN(31),
	SERVALT_PIN(32),
	SERVALT_PIN(33),
	SERVALT_PIN(34),
	SERVALT_PIN(35),
	SERVALT_PIN(36),
};

static const unsigned long servalt_gpios[] = {
	[MSCC_GPIO_OUT_SET] = 0x00,
	[MSCC_GPIO_OUT_CLR] = 0x08,
	[MSCC_GPIO_OUT] = 0x10,
	[MSCC_GPIO_IN] = 0x18,
	[MSCC_GPIO_OE] = 0x20,
	[MSCC_GPIO_INTR] = 0x28,
	[MSCC_GPIO_INTR_ENA] = 0x30,
	[MSCC_GPIO_INTR_IDENT] = 0x38,
	[MSCC_GPIO_ALT0] = 0x40,
	[MSCC_GPIO_ALT1] = 0x48,
};

static int servalt_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "servalt-gpio";
	uc_priv->gpio_count = ARRAY_SIZE(servalt_pins);

	return 0;
}

static struct driver servalt_gpio_driver = {
	.name	= "servalt-gpio",
	.id	= UCLASS_GPIO,
	.probe	= servalt_gpio_probe,
	.ops	= &mscc_gpio_ops,
};

static int servalt_pinctrl_probe(struct udevice *dev)
{
	int ret;

	ret = mscc_pinctrl_probe(dev, FUNC_MAX, servalt_pins,
				 ARRAY_SIZE(servalt_pins),
				 servalt_function_names,
				 servalt_gpios);

	if (ret)
		return ret;

	ret = device_bind(dev, &servalt_gpio_driver, "servalt-gpio", NULL,
			  dev_ofnode(dev), NULL);

	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id servalt_pinctrl_of_match[] = {
	{ .compatible = "mscc,servalt-pinctrl" },
	{},
};

U_BOOT_DRIVER(servalt_pinctrl) = {
	.name = "servalt-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(servalt_pinctrl_of_match),
	.probe = servalt_pinctrl_probe,
	.priv_auto	= sizeof(struct mscc_pinctrl),
	.ops = &mscc_pinctrl_ops,
};
