// SPDX-License-Identifier: GPL-2.0+

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <mach/jz4780.h>

int jz47xx_gpio_get_value(unsigned int gpio)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;
	int port = gpio / 32;
	int pin = gpio % 32;

	return readl(gpio_regs + GPIO_PXPIN(port)) & BIT(pin);
}

void jz47xx_gpio_direction_input(unsigned int gpio)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;
	int port = gpio / 32;
	int pin = gpio % 32;

	writel(BIT(pin), gpio_regs + GPIO_PXINTC(port));
	writel(BIT(pin), gpio_regs + GPIO_PXMASKS(port));
	writel(BIT(pin), gpio_regs + GPIO_PXPAT1S(port));
}

void jz47xx_gpio_direction_output(unsigned int gpio, int value)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;
	int port = gpio / 32;
	int pin = gpio % 32;

	writel(BIT(pin), gpio_regs + GPIO_PXINTC(port));
	writel(BIT(pin), gpio_regs + GPIO_PXMASKS(port));
	writel(BIT(pin), gpio_regs + GPIO_PXPAT1C(port));
	writel(BIT(pin), gpio_regs +
			 (value ? GPIO_PXPAT0S(port) : GPIO_PXPAT0C(port)));
}
