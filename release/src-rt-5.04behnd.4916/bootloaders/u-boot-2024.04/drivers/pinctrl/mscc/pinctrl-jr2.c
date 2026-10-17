// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <horatiu.vultur@microchip.com>
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
	FUNC_SFP11,
	FUNC_SFP12,
	FUNC_SFP13,
	FUNC_SFP14,
	FUNC_SFP15,
	FUNC_SG0,
	FUNC_SG1,
	FUNC_SG2,
	FUNC_SI,
	FUNC_TACHO,
	FUNC_TWI,
	FUNC_TWI2,
	FUNC_TWI_SCL_M,
	FUNC_UART,
	FUNC_UART2,
	FUNC_MAX
};

static char * const jr2_function_names[] = {
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
	[FUNC_SFP11]		= "sfp11",
	[FUNC_SFP12]		= "sfp12",
	[FUNC_SFP13]		= "sfp13",
	[FUNC_SFP14]		= "sfp14",
	[FUNC_SFP15]		= "sfp15",
	[FUNC_SG0]		= "sg0",
	[FUNC_SG1]		= "sg1",
	[FUNC_SG2]		= "sg2",
	[FUNC_SI]		= "si",
	[FUNC_TACHO]		= "tacho",
	[FUNC_TWI]		= "twi",
	[FUNC_TWI2]		= "twi2",
	[FUNC_TWI_SCL_M]	= "twi_scl_m",
	[FUNC_UART]		= "uart",
	[FUNC_UART2]		= "uart2",
};

#define JR2_P(p, f0, f1)						\
static struct mscc_pin_caps jr2_pin_##p = {				\
	.pin = p,							\
	.functions = {							\
			FUNC_GPIO, FUNC_##f0, FUNC_##f1, FUNC_NONE	\
	},								\
}

JR2_P(0,  SG0,       NONE);
JR2_P(1,  SG0,       NONE);
JR2_P(2,  SG0,       NONE);
JR2_P(3,  SG0,       NONE);
JR2_P(4,  SG1,       NONE);
JR2_P(5,  SG1,       NONE);
JR2_P(6,  IRQ0_IN,   IRQ0_OUT);
JR2_P(7,  IRQ1_IN,   IRQ1_OUT);
JR2_P(8,  PTP0,      NONE);
JR2_P(9,  PTP1,      NONE);
JR2_P(10, UART,      NONE);
JR2_P(11, UART,      NONE);
JR2_P(12, SG1,       NONE);
JR2_P(13, SG1,       NONE);
JR2_P(14, TWI,       TWI_SCL_M);
JR2_P(15, TWI,       NONE);
JR2_P(16, SI,        TWI_SCL_M);
JR2_P(17, SI,        TWI_SCL_M);
JR2_P(18, SI,        TWI_SCL_M);
JR2_P(19, PCI_WAKE,  NONE);
JR2_P(20, IRQ0_OUT,  TWI_SCL_M);
JR2_P(21, IRQ1_OUT,  TWI_SCL_M);
JR2_P(22, TACHO,     NONE);
JR2_P(23, PWM,       NONE);
JR2_P(24, UART2,     NONE);
JR2_P(25, UART2,     SI);
JR2_P(26, PTP2,      SI);
JR2_P(27, PTP3,      SI);
JR2_P(28, TWI2,      SI);
JR2_P(29, TWI,       SI);
JR2_P(30, SG2,       SI);
JR2_P(31, SG2,       SI);
JR2_P(32, SG2,       SI);
JR2_P(33, SG2,       SI);
JR2_P(34, NONE,      TWI_SCL_M);
JR2_P(35, NONE,      TWI_SCL_M);
JR2_P(36, NONE,      TWI_SCL_M);
JR2_P(37, NONE,      TWI_SCL_M);
JR2_P(38, NONE,      TWI_SCL_M);
JR2_P(39, NONE,      TWI_SCL_M);
JR2_P(40, NONE,      TWI_SCL_M);
JR2_P(41, NONE,      TWI_SCL_M);
JR2_P(42, NONE,      TWI_SCL_M);
JR2_P(43, NONE,      TWI_SCL_M);
JR2_P(44, NONE,      SFP8);
JR2_P(45, NONE,      SFP9);
JR2_P(46, NONE,      SFP10);
JR2_P(47, NONE,      SFP11);
JR2_P(48, SFP0,      NONE);
JR2_P(49, SFP1,      SI);
JR2_P(50, SFP2,      SI);
JR2_P(51, SFP3,      SI);
JR2_P(52, SFP4,      NONE);
JR2_P(53, SFP5,      NONE);
JR2_P(54, SFP6,      NONE);
JR2_P(55, SFP7,      NONE);
JR2_P(56, MIIM1,     SFP12);
JR2_P(57, MIIM1,     SFP13);
JR2_P(58, MIIM2,     SFP14);
JR2_P(59, MIIM2,     SFP15);
JR2_P(60, NONE,      NONE);
JR2_P(61, NONE,      NONE);
JR2_P(62, NONE,      NONE);
JR2_P(63, NONE,      NONE);

#define JR2_PIN(n) {						\
	.name = "GPIO_"#n,					\
	.drv_data = &jr2_pin_##n				\
}

static const struct mscc_pin_data jr2_pins[] = {
	JR2_PIN(0),
	JR2_PIN(1),
	JR2_PIN(2),
	JR2_PIN(3),
	JR2_PIN(4),
	JR2_PIN(5),
	JR2_PIN(6),
	JR2_PIN(7),
	JR2_PIN(8),
	JR2_PIN(9),
	JR2_PIN(10),
	JR2_PIN(11),
	JR2_PIN(12),
	JR2_PIN(13),
	JR2_PIN(14),
	JR2_PIN(15),
	JR2_PIN(16),
	JR2_PIN(17),
	JR2_PIN(18),
	JR2_PIN(19),
	JR2_PIN(20),
	JR2_PIN(21),
	JR2_PIN(22),
	JR2_PIN(23),
	JR2_PIN(24),
	JR2_PIN(25),
	JR2_PIN(26),
	JR2_PIN(27),
	JR2_PIN(28),
	JR2_PIN(29),
	JR2_PIN(30),
	JR2_PIN(31),
	JR2_PIN(32),
	JR2_PIN(33),
	JR2_PIN(34),
	JR2_PIN(35),
	JR2_PIN(36),
	JR2_PIN(37),
	JR2_PIN(38),
	JR2_PIN(39),
	JR2_PIN(40),
	JR2_PIN(41),
	JR2_PIN(42),
	JR2_PIN(43),
	JR2_PIN(44),
	JR2_PIN(45),
	JR2_PIN(46),
	JR2_PIN(47),
	JR2_PIN(48),
	JR2_PIN(49),
	JR2_PIN(50),
	JR2_PIN(51),
	JR2_PIN(52),
	JR2_PIN(53),
	JR2_PIN(54),
	JR2_PIN(55),
	JR2_PIN(56),
	JR2_PIN(57),
	JR2_PIN(58),
	JR2_PIN(59),
	JR2_PIN(60),
	JR2_PIN(61),
	JR2_PIN(62),
	JR2_PIN(63),
};

static const unsigned long jr2_gpios[] = {
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

static int jr2_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = "jr2-gpio";
	uc_priv->gpio_count = ARRAY_SIZE(jr2_pins);

	return 0;
}

static struct driver jr2_gpio_driver = {
	.name	= "jr2-gpio",
	.id	= UCLASS_GPIO,
	.probe	= jr2_gpio_probe,
	.ops	= &mscc_gpio_ops,
};

static int jr2_pinctrl_probe(struct udevice *dev)
{
	int ret;

	ret = mscc_pinctrl_probe(dev, FUNC_MAX, jr2_pins,
				 ARRAY_SIZE(jr2_pins),
				 jr2_function_names,
				 jr2_gpios);

	if (ret)
		return ret;

	ret = device_bind(dev, &jr2_gpio_driver, "jr2-gpio", NULL,
			  dev_ofnode(dev), NULL);

	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id jr2_pinctrl_of_match[] = {
	{ .compatible = "mscc,jaguar2-pinctrl" },
	{},
};

U_BOOT_DRIVER(jr2_pinctrl) = {
	.name = "jr2-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(jr2_pinctrl_of_match),
	.probe = jr2_pinctrl_probe,
	.priv_auto	= sizeof(struct mscc_pinctrl),
	.ops = &mscc_pinctrl_ops,
};
