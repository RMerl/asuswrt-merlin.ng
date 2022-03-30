// SPDX-License-Identifier: GPL-2.0+
/*
 * cpld-gpio-bus.c: provides support for the CPLD GPIO bus found on some LaCie
 * boards (as the 2Big/5Big Network v2 and the 2Big NAS). This parallel GPIO
 * bus exposes two registers (address and data). Each of this register is made
 * up of several dedicated GPIOs. An extra GPIO is used to notify the CPLD that
 * the registers have been updated.
 *
 * Mostly this bus is used to configure the LEDs on LaCie boards.
 *
 * Copyright (C) 2013 Simon Guinot <simon.guinot@sequanux.org>
 */

#include <asm/arch/gpio.h>
#include "cpld-gpio-bus.h"

static void cpld_gpio_bus_set_addr(struct cpld_gpio_bus *bus, unsigned addr)
{
	int pin;

	for (pin = 0; pin < bus->num_addr; pin++)
		kw_gpio_set_value(bus->addr[pin], (addr >> pin) & 1);
}

static void cpld_gpio_bus_set_data(struct cpld_gpio_bus *bus, unsigned data)
{
	int pin;

	for (pin = 0; pin < bus->num_data; pin++)
		kw_gpio_set_value(bus->data[pin], (data >> pin) & 1);
}

static void cpld_gpio_bus_enable_select(struct cpld_gpio_bus *bus)
{
	/* The transfer is enabled on the raising edge. */
	kw_gpio_set_value(bus->enable, 0);
	kw_gpio_set_value(bus->enable, 1);
}

void cpld_gpio_bus_write(struct cpld_gpio_bus *bus,
			 unsigned addr, unsigned value)
{
	cpld_gpio_bus_set_addr(bus, addr);
	cpld_gpio_bus_set_data(bus, value);
	cpld_gpio_bus_enable_select(bus);
}
