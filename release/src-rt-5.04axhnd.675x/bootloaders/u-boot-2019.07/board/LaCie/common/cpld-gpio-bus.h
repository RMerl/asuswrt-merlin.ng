/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Simon Guinot <simon.guinot@sequanux.org>
 */

#ifndef _LACIE_CPLD_GPI0_BUS_H
#define _LACIE_CPLD_GPI0_BUS_H

struct cpld_gpio_bus {
	unsigned *addr;
	unsigned num_addr;
	unsigned *data;
	unsigned num_data;
	unsigned enable;
};

void cpld_gpio_bus_write(struct cpld_gpio_bus *cpld_gpio_bus,
			 unsigned addr, unsigned value);

#endif /* _LACIE_CPLD_GPI0_BUS_H */
