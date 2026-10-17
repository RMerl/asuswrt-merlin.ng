/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __ACPI_I2C_H
#define __ACPI_I2C_H

#include <dm/acpi.h>

extern struct acpi_ops acpi_i2c_ops;

int acpi_i2c_of_to_plat(struct udevice *dev);

#endif
