/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __DT_HELPERS_H
#define __DT_HELPERS_H

int fdt_disable_by_ofname(void *rw_fdt_blob, char *ofname);
bool dm_i2c_simple_probe(struct udevice *bus, uint chip_addr);
int request_gpio_by_name(struct gpio_desc *gpio, const char *gpio_dev_name,
			 uint offset, char *gpio_name);

#endif /* __DT_HELPERS_H */
