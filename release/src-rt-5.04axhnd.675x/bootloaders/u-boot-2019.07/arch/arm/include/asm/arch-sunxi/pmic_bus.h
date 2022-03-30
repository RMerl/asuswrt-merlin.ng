/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Sunxi PMIC bus access helpers header
 */

#ifndef _SUNXI_PMIC_BUS_H
#define _SUNXI_PMIS_BUS_H

int pmic_bus_init(void);
int pmic_bus_read(u8 reg, u8 *data);
int pmic_bus_write(u8 reg, u8 data);
int pmic_bus_setbits(u8 reg, u8 bits);
int pmic_bus_clrbits(u8 reg, u8 bits);

#endif
