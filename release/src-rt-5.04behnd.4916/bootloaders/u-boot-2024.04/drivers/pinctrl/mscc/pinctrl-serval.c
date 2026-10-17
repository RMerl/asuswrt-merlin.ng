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
	FUNC_IRQ0,
	FUNC_IRQ1,
	FUNC_MIIM1,
	FUNC_PCI_WAKE,
	FUNC_PTP0,
	FUNC_PTP1,
	FUNC_PTP2,
	FUNC_PTP3,
	FUNC_PWM,
	FUNC_RECO_CLK0,
	FUNC_RECO_CLK1,
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
	FUNC_SIO,
	FUNC_SI,
	FUNC_TACHO,
	FUNC_TWI,
	FUNC_TWI_SCL_M,
	FUNC_UART,
	FUNC_UART2,
	FUNC_MD,
	FUNC_PTP1588,
	FUNC_MAX
};

static char * const serval_function_names[] = {
	[FUNC_NONE]		= "none",
	[FUNC_GPIO]		= "gpio",
	[FUNC_IRQ0]		= "irq0",
	[FUNC_IRQ1]		= "irq1",
	[FUNC_MIIM1]		= "miim1",
	[FUNC_PCI_WAKE]		= "pci_wake",
	[FUNC_PTP0]		= "ptp0",
	[FUNC_PTP1]		= "ptp1",
	[FUNC_PTP2]		= "ptp2",
	[FUNC_PTP3]		= "ptp3",
	[FUNC_PWM]		= "pwm",
	[FUNC_RECO_CLK0]	= "reco_clk0",
	[FUNC_RECO_CLK1]	= "reco_clk1",
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
	[FUNC_SIO]		= "sio",
	[FUNC_SI]		= "si",
	[FUNC_TACHO]		= "tacho",
	[FUNC_TWI]		= "twi",
	[FUNC_TWI_SCL_M]	= "twi_scl_m",
	[FUNC_UART]		= "uart",
	[FUNC_UART2]		= "uart2",
	[FUNC_MD]		= "md",
	[FUNC_PTP1588]		= "1588",
};

MSCC_P(0,  SIO,       NONE,      NONE);
MSCC_P(1,  SIO,       NONE,      NONE);
MSCC_P(2,  SIO,       NONE,      NONE);
MSCC_P(3,  SIO,       NONE,      NONE);
MSCC_P(4,  TACHO,     NONE,      NONE);
MSCC_P(5,  PWM,       NONE,      NONE);
MSCC_P(6,  TWI,       NONE,      NONE);
MSCC_P(7,  TWI,       NONE,      NONE);
MSCC_P(8,  SI,        NONE,      NONE);
MSCC_P(9,  SI,        MD,        NONE);
MSCC_P(10, SI,        MD,        NONE);
MSCC_P(11, SFP0,      MD,        TWI_SCL_M);
MSCC_P(12, SFP1,      MD,        TWI_SCL_M);
MSCC_P(13, SFP2,      UART2,     TWI_SCL_M);
MSCC_P(14, SFP3,      UART2,     TWI_SCL_M);
MSCC_P(15, SFP4,      PTP1588,   TWI_SCL_M);
MSCC_P(16, SFP5,      PTP1588,   TWI_SCL_M);
MSCC_P(17, SFP6,      PCI_WAKE,  TWI_SCL_M);
MSCC_P(18, SFP7,      NONE,      TWI_SCL_M);
MSCC_P(19, SFP8,      NONE,      TWI_SCL_M);
MSCC_P(20, SFP9,      NONE,      TWI_SCL_M);
MSCC_P(21, SFP10,     NONE,      TWI_SCL_M);
MSCC_P(22, NONE,      NONE,      NONE);
MSCC_P(23, NONE,      NONE,      NONE);
MSCC_P(24, NONE,      NONE,      NONE);
MSCC_P(25, NONE,      NONE,      NONE);
MSCC_P(26, UART,      NONE,      NONE);
MSCC_P(27, UART,      NONE,      NONE);
MSCC_P(28, IRQ0,      NONE,      NONE);
MSCC_P(29, IRQ1,      NONE,      NONE);
MSCC_P(30, PTP1588,   NONE,      NONE);
MSCC_P(31, PTP1588,   NONE,      NONE);

#define SERVAL_PIN(n) {						\
	.name = "GPIO_"#n,					\
	.drv_data = &mscc_pin_##n				\
}

static const struct mscc_pin_data serval_pins[] = {
	SERVAL_PIN(0),
	SERVAL_PIN(1),
	SERVAL_PIN(2),
	SERVAL_PIN(3),
	SERVAL_PIN(4),
	SERVAL_PIN(5),
	SERVAL_PIN(6),
	SERVAL_PIN(7),
	SERVAL_PIN(8),
	SERVAL_PIN(9),
	SERVAL_PIN(10),
	SERVAL_PIN(11),
	SERVAL_PIN(12),
	SERVAL_PIN(13),
	SERVAL_PIN(14),
	SERVAL_PIN(15),
	SERVAL_PIN(16),
	SERVAL_PIN(17),
	SERVAL_PIN(18),
	SERVAL_PIN(19),
	SERVAL_PIN(20),
	SERVAL_PIN(21),
	SERVAL_PIN(22),
	SERVAL_PIN(23),
	SERVAL_PIN(24),
	SERVAL_PIN(25),
	SERVAL_PIN(26),
	SERVAL_PIN(27),
	SERVAL_PIN(28),
	SERVAL_PIN(29),
	SERVAL_PIN(30),
	SERVAL_PIN(31),
};

static const unsigned long serval_gpios[] = {
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

static int serval_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "serval-gpio";
	uc_priv->gpio_count = ARRAY_SIZE(serval_pins);

	return 0;
}

static struct driver serval_gpio_driver = {
	.name	= "serval-gpio",
	.id	= UCLASS_GPIO,
	.probe	= serval_gpio_probe,
	.ops	= &mscc_gpio_ops,
};

static int serval_pinctrl_probe(struct udevice *dev)
{
	int ret;

	ret = mscc_pinctrl_probe(dev, FUNC_MAX, serval_pins,
				 ARRAY_SIZE(serval_pins),
				 serval_function_names,
				 serval_gpios);

	if (ret)
		return ret;

	ret = device_bind(dev, &serval_gpio_driver, "serval-gpio", NULL,
			  dev_ofnode(dev), NULL);

	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id serval_pinctrl_of_match[] = {
	{ .compatible = "mscc,serval-pinctrl" },
	{},
};

U_BOOT_DRIVER(serval_pinctrl) = {
	.name = "serval-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(serval_pinctrl_of_match),
	.probe = serval_pinctrl_probe,
	.priv_auto	= sizeof(struct mscc_pinctrl),
	.ops = &mscc_pinctrl_ops,
};
