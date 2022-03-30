// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Bo Shen <voice.shen@atmel.com>
 *
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 *  Copyright (C) 2005 HP Labs
 */

#include <config.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <asm/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>

#define GPIO_PER_BANK	32

static struct at91_port *at91_pio_get_port(unsigned port)
{
	switch (port) {
	case AT91_PIO_PORTA:
		return (struct at91_port *)ATMEL_BASE_PIOA;
	case AT91_PIO_PORTB:
		return (struct at91_port *)ATMEL_BASE_PIOB;
	case AT91_PIO_PORTC:
		return (struct at91_port *)ATMEL_BASE_PIOC;
#if (ATMEL_PIO_PORTS > 3)
	case AT91_PIO_PORTD:
		return (struct at91_port *)ATMEL_BASE_PIOD;
#if (ATMEL_PIO_PORTS > 4)
	case AT91_PIO_PORTE:
		return (struct at91_port *)ATMEL_BASE_PIOE;
#endif
#endif
	default:
		printf("Error: at91_gpio: Fail to get PIO base!\n");
		return NULL;
	}
}

static void at91_set_port_pullup(struct at91_port *at91_port, unsigned offset,
				 int use_pullup)
{
	u32 mask;

	mask = 1 << offset;
	if (use_pullup)
		writel(mask, &at91_port->puer);
	else
		writel(mask, &at91_port->pudr);
	writel(mask, &at91_port->per);
}

int at91_set_pio_pullup(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (at91_port && (pin < GPIO_PER_BANK))
		at91_set_port_pullup(at91_port, pin, use_pullup);

	return 0;
}

/*
 * mux the pin to the "GPIO" peripheral role.
 */
int at91_set_pio_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(mask, &at91_port->per);
	}

	return 0;
}

/*
 * mux the pin to the "A" internal peripheral role.
 */
int at91_set_a_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(mask, &at91_port->mux.pio2.asr);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "B" internal peripheral role.
 */
int at91_set_b_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(mask, &at91_port->mux.pio2.bsr);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "A" internal peripheral role.
 */
int at91_pio3_set_a_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->mux.pio3.abcdsr1) & ~mask,
		       &at91_port->mux.pio3.abcdsr1);
		writel(readl(&at91_port->mux.pio3.abcdsr2) & ~mask,
		       &at91_port->mux.pio3.abcdsr2);

		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "B" internal peripheral role.
 */
int at91_pio3_set_b_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->mux.pio3.abcdsr1) | mask,
		       &at91_port->mux.pio3.abcdsr1);
		writel(readl(&at91_port->mux.pio3.abcdsr2) & ~mask,
		       &at91_port->mux.pio3.abcdsr2);

		writel(mask, &at91_port->pdr);
	}

	return 0;
}
/*
 * mux the pin to the "C" internal peripheral role.
 */
int at91_pio3_set_c_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->mux.pio3.abcdsr1) & ~mask,
		       &at91_port->mux.pio3.abcdsr1);
		writel(readl(&at91_port->mux.pio3.abcdsr2) | mask,
		       &at91_port->mux.pio3.abcdsr2);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "D" internal peripheral role.
 */
int at91_pio3_set_d_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->mux.pio3.abcdsr1) | mask,
		       &at91_port->mux.pio3.abcdsr1);
		writel(readl(&at91_port->mux.pio3.abcdsr2) | mask,
		       &at91_port->mux.pio3.abcdsr2);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

#ifdef CONFIG_DM_GPIO
static bool at91_get_port_output(struct at91_port *at91_port, int offset)
{
	u32 mask, val;

	mask = 1 << offset;
	val = readl(&at91_port->osr);
	return val & mask;
}
#endif

static void at91_set_port_input(struct at91_port *at91_port, int offset,
				int use_pullup)
{
	u32 mask;

	mask = 1 << offset;
	writel(mask, &at91_port->idr);
	at91_set_port_pullup(at91_port, offset, use_pullup);
	writel(mask, &at91_port->odr);
	writel(mask, &at91_port->per);
}

/*
 * mux the pin to the gpio controller (instead of "A" or "B" peripheral), and
 * configure it for an input.
 */
int at91_set_pio_input(unsigned port, u32 pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (at91_port && (pin < GPIO_PER_BANK))
		at91_set_port_input(at91_port, pin, use_pullup);

	return 0;
}

static void at91_set_port_output(struct at91_port *at91_port, int offset,
				 int value)
{
	u32 mask;

	mask = 1 << offset;
	writel(mask, &at91_port->idr);
	writel(mask, &at91_port->pudr);
	if (value)
		writel(mask, &at91_port->sodr);
	else
		writel(mask, &at91_port->codr);
	writel(mask, &at91_port->oer);
	writel(mask, &at91_port->per);
}

/*
 * mux the pin to the gpio controller (instead of "A" or "B" peripheral),
 * and configure it for an output.
 */
int at91_set_pio_output(unsigned port, u32 pin, int value)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (at91_port && (pin < GPIO_PER_BANK))
		at91_set_port_output(at91_port, pin, value);

	return 0;
}

/*
 * enable/disable the glitch filter. mostly used with IRQ handling.
 */
int at91_set_pio_deglitch(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		if (is_on)
			writel(mask, &at91_port->ifer);
		else
			writel(mask, &at91_port->ifdr);
	}

	return 0;
}

/*
 * enable/disable the glitch filter. mostly used with IRQ handling.
 */
int at91_pio3_set_pio_deglitch(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		if (is_on) {
			writel(mask, &at91_port->mux.pio3.ifscdr);
			writel(mask, &at91_port->ifer);
		} else {
			writel(mask, &at91_port->ifdr);
		}
	}

	return 0;
}

/*
 * enable/disable the debounce filter.
 */
int at91_pio3_set_pio_debounce(unsigned port, unsigned pin, int is_on, int div)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		if (is_on) {
			writel(mask, &at91_port->mux.pio3.ifscer);
			writel(div & PIO_SCDR_DIV, &at91_port->mux.pio3.scdr);
			writel(mask, &at91_port->ifer);
		} else {
			writel(mask, &at91_port->ifdr);
		}
	}

	return 0;
}

/*
 * enable/disable the pull-down.
 * If pull-up already enabled while calling the function, we disable it.
 */
int at91_pio3_set_pio_pulldown(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		if (is_on) {
			at91_set_pio_pullup(port, pin, 0);
			writel(mask, &at91_port->mux.pio3.ppder);
		} else
			writel(mask, &at91_port->mux.pio3.ppddr);
	}

	return 0;
}

int at91_pio3_set_pio_pullup(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (use_pullup)
		at91_pio3_set_pio_pulldown(port, pin, 0);

	if (at91_port && (pin < GPIO_PER_BANK))
		at91_set_port_pullup(at91_port, pin, use_pullup);

	return 0;
}

/*
 * disable Schmitt trigger
 */
int at91_pio3_set_pio_disable_schmitt_trig(unsigned port, unsigned pin)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		writel(readl(&at91_port->schmitt) | mask,
		       &at91_port->schmitt);
	}

	return 0;
}

/*
 * enable/disable the multi-driver. This is only valid for output and
 * allows the output pin to run as an open collector output.
 */
int at91_set_pio_multi_drive(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < GPIO_PER_BANK)) {
		mask = 1 << pin;
		if (is_on)
			writel(mask, &at91_port->mder);
		else
			writel(mask, &at91_port->mddr);
	}

	return 0;
}

static void at91_set_port_value(struct at91_port *at91_port, int offset,
				int value)
{
	u32 mask;

	mask = 1 << offset;
	if (value)
		writel(mask, &at91_port->sodr);
	else
		writel(mask, &at91_port->codr);
}

/*
 * assuming the pin is muxed as a gpio output, set its value.
 */
int at91_set_pio_value(unsigned port, unsigned pin, int value)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (at91_port && (pin < GPIO_PER_BANK))
		at91_set_port_value(at91_port, pin, value);

	return 0;
}

static int at91_get_port_value(struct at91_port *at91_port, int offset)
{
	u32 pdsr = 0, mask;

	mask = 1 << offset;
	pdsr = readl(&at91_port->pdsr) & mask;

	return pdsr != 0;
}
/*
 * read the pin's value (works even if it's not muxed as a gpio).
 */
int at91_get_pio_value(unsigned port, unsigned pin)
{
	struct at91_port *at91_port = at91_pio_get_port(port);

	if (at91_port && (pin < GPIO_PER_BANK))
		return at91_get_port_value(at91_port, pin);

	return 0;
}

#ifndef CONFIG_DM_GPIO
/* Common GPIO API */

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	at91_set_pio_input(at91_gpio_to_port(gpio),
			   at91_gpio_to_pin(gpio), 0);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	at91_set_pio_output(at91_gpio_to_port(gpio),
			    at91_gpio_to_pin(gpio), value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	return at91_get_pio_value(at91_gpio_to_port(gpio),
				  at91_gpio_to_pin(gpio));
}

int gpio_set_value(unsigned gpio, int value)
{
	at91_set_pio_value(at91_gpio_to_port(gpio),
			   at91_gpio_to_pin(gpio), value);

	return 0;
}
#endif

#ifdef CONFIG_DM_GPIO

struct at91_port_priv {
	struct at91_port *regs;
};

/* set GPIO pin 'gpio' as an input */
static int at91_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct at91_port_priv *port = dev_get_priv(dev);

	at91_set_port_input(port->regs, offset, 0);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int at91_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct at91_port_priv *port = dev_get_priv(dev);

	at91_set_port_output(port->regs, offset, value);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int at91_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct at91_port_priv *port = dev_get_priv(dev);

	return at91_get_port_value(port->regs, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int at91_gpio_set_value(struct udevice *dev, unsigned offset,
			       int value)
{
	struct at91_port_priv *port = dev_get_priv(dev);

	at91_set_port_value(port->regs, offset, value);

	return 0;
}

static int at91_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct at91_port_priv *port = dev_get_priv(dev);

	/* GPIOF_FUNC is not implemented yet */
	if (at91_get_port_output(port->regs, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_at91_ops = {
	.direction_input	= at91_gpio_direction_input,
	.direction_output	= at91_gpio_direction_output,
	.get_value		= at91_gpio_get_value,
	.set_value		= at91_gpio_set_value,
	.get_function		= at91_gpio_get_function,
};

static int at91_gpio_probe(struct udevice *dev)
{
	struct at91_port_priv *port = dev_get_priv(dev);
	struct at91_port_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_free(&clk);

	uc_priv->bank_name = plat->bank_name;
	uc_priv->gpio_count = GPIO_PER_BANK;

#if CONFIG_IS_ENABLED(OF_CONTROL)
	plat->base_addr = (uint32_t)devfdt_get_addr_ptr(dev);
#endif
	port->regs = (struct at91_port *)plat->base_addr;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id at91_gpio_ids[] = {
	{ .compatible = "atmel,at91rm9200-gpio" },
	{ }
};
#endif

U_BOOT_DRIVER(gpio_at91) = {
	.name	= "gpio_at91",
	.id	= UCLASS_GPIO,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = at91_gpio_ids,
	.platdata_auto_alloc_size = sizeof(struct at91_port_platdata),
#endif
	.ops	= &gpio_at91_ops,
	.probe	= at91_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct at91_port_priv),
};
#endif
