// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <alexandre.belloni@free-electrons.com>
 * Author: <gregory.clement@bootlin.com>
 * License: Dual MIT/GPL
 * Copyright (c) 2017 Microsemi Corporation
 */

#include <asm/gpio.h>
#include <asm/system.h>
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
#include "mscc-common.h"

enum {
	FUNC_NONE,
	FUNC_GPIO,
	FUNC_IRQ0_IN,
	FUNC_IRQ0_OUT,
	FUNC_IRQ1_IN,
	FUNC_IRQ1_OUT,
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
	FUNC_SG0,
	FUNC_SI,
	FUNC_TACHO,
	FUNC_TWI,
	FUNC_TWI_SCL_M,
	FUNC_UART,
	FUNC_UART2,
	FUNC_MAX
};

static char * const ocelot_function_names[] = {
	[FUNC_NONE]		= "none",
	[FUNC_GPIO]		= "gpio",
	[FUNC_IRQ0_IN]		= "irq0_in",
	[FUNC_IRQ0_OUT]		= "irq0_out",
	[FUNC_IRQ1_IN]		= "irq1_in",
	[FUNC_IRQ1_OUT]		= "irq1_out",
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
	[FUNC_SG0]		= "sg0",
	[FUNC_SI]		= "si",
	[FUNC_TACHO]		= "tacho",
	[FUNC_TWI]		= "twi",
	[FUNC_TWI_SCL_M]	= "twi_scl_m",
	[FUNC_UART]		= "uart",
	[FUNC_UART2]		= "uart2",
};

MSCC_P(0,  SG0,       NONE,      NONE);
MSCC_P(1,  SG0,       NONE,      NONE);
MSCC_P(2,  SG0,       NONE,      NONE);
MSCC_P(3,  SG0,       NONE,      NONE);
MSCC_P(4,  IRQ0_IN,   IRQ0_OUT,  TWI);
MSCC_P(5,  IRQ1_IN,   IRQ1_OUT,  PCI_WAKE);
MSCC_P(6,  UART,      TWI_SCL_M, NONE);
MSCC_P(7,  UART,      TWI_SCL_M, NONE);
MSCC_P(8,  SI,        TWI_SCL_M, IRQ0_OUT);
MSCC_P(9,  SI,        TWI_SCL_M, IRQ1_OUT);
MSCC_P(10, PTP2,      TWI_SCL_M, SFP0);
MSCC_P(11, PTP3,      TWI_SCL_M, SFP1);
MSCC_P(12, UART2,     TWI_SCL_M, SFP2);
MSCC_P(13, UART2,     TWI_SCL_M, SFP3);
MSCC_P(14, MIIM1,     TWI_SCL_M, SFP4);
MSCC_P(15, MIIM1,     TWI_SCL_M, SFP5);
MSCC_P(16, TWI,       NONE,      SI);
MSCC_P(17, TWI,       TWI_SCL_M, SI);
MSCC_P(18, PTP0,      TWI_SCL_M, NONE);
MSCC_P(19, PTP1,      TWI_SCL_M, NONE);
MSCC_P(20, RECO_CLK0, TACHO,     NONE);
MSCC_P(21, RECO_CLK1, PWM,       NONE);

#define OCELOT_PIN(n) {						\
	.name = "GPIO_"#n,					\
	.drv_data = &mscc_pin_##n				\
}

static const struct mscc_pin_data ocelot_pins[] = {
	OCELOT_PIN(0),
	OCELOT_PIN(1),
	OCELOT_PIN(2),
	OCELOT_PIN(3),
	OCELOT_PIN(4),
	OCELOT_PIN(5),
	OCELOT_PIN(6),
	OCELOT_PIN(7),
	OCELOT_PIN(8),
	OCELOT_PIN(9),
	OCELOT_PIN(10),
	OCELOT_PIN(11),
	OCELOT_PIN(12),
	OCELOT_PIN(13),
	OCELOT_PIN(14),
	OCELOT_PIN(15),
	OCELOT_PIN(16),
	OCELOT_PIN(17),
	OCELOT_PIN(18),
	OCELOT_PIN(19),
	OCELOT_PIN(20),
	OCELOT_PIN(21),
};

static const unsigned long ocelot_gpios[] = {
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

static int ocelot_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "ocelot-gpio";
	uc_priv->gpio_count = ARRAY_SIZE(ocelot_pins);

	return 0;
}

static struct driver ocelot_gpio_driver = {
	.name	= "ocelot-gpio",
	.id	= UCLASS_GPIO,
	.probe	= ocelot_gpio_probe,
	.ops	= &mscc_gpio_ops,
};

int ocelot_pinctrl_probe(struct udevice *dev)
{
	int ret;

	ret = mscc_pinctrl_probe(dev, FUNC_MAX, ocelot_pins,
				 ARRAY_SIZE(ocelot_pins),
				 ocelot_function_names,
				 ocelot_gpios);

	if (ret)
		return ret;

	ret = device_bind(dev, &ocelot_gpio_driver, "ocelot-gpio", NULL,
			  dev_of_offset(dev), NULL);

	return ret;
}

static const struct udevice_id ocelot_pinctrl_of_match[] = {
	{.compatible = "mscc,ocelot-pinctrl"},
	{},
};

U_BOOT_DRIVER(ocelot_pinctrl) = {
	.name = "ocelot-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(ocelot_pinctrl_of_match),
	.probe = ocelot_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct mscc_pinctrl),
	.ops = &mscc_pinctrl_ops,
};
