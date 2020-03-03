/*
 * intel_pmic.c - Intel PMIC operation region driver
 *
 * Copyright (C) 2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/acpi.h>
#include <linux/regmap.h>
#include <acpi/acpi_lpat.h>
#include "intel_pmic.h"

#define PMIC_POWER_OPREGION_ID		0x8d
#define PMIC_THERMAL_OPREGION_ID	0x8c

struct intel_pmic_opregion {
	struct mutex lock;
	struct acpi_lpat_conversion_table *lpat_table;
	struct regmap *regmap;
	struct intel_pmic_opregion_data *data;
};

static int pmic_get_reg_bit(int address, struct pmic_table *table,
			    int count, int *reg, int *bit)
{
	int i;

	for (i = 0; i < count; i++) {
		if (table[i].address == address) {
			*reg = table[i].reg;
			if (bit)
				*bit = table[i].bit;
			return 0;
		}
	}
	return -ENOENT;
}

static acpi_status intel_pmic_power_handler(u32 function,
		acpi_physical_address address, u32 bits, u64 *value64,
		void *handler_context, void *region_context)
{
	struct intel_pmic_opregion *opregion = region_context;
	struct regmap *regmap = opregion->regmap;
	struct intel_pmic_opregion_data *d = opregion->data;
	int reg, bit, result;

	if (bits != 32 || !value64)
		return AE_BAD_PARAMETER;

	if (function == ACPI_WRITE && !(*value64 == 0 || *value64 == 1))
		return AE_BAD_PARAMETER;

	result = pmic_get_reg_bit(address, d->power_table,
				  d->power_table_count, &reg, &bit);
	if (result == -ENOENT)
		return AE_BAD_PARAMETER;

	mutex_lock(&opregion->lock);

	result = function == ACPI_READ ?
		d->get_power(regmap, reg, bit, value64) :
		d->update_power(regmap, reg, bit, *value64 == 1);

	mutex_unlock(&opregion->lock);

	return result ? AE_ERROR : AE_OK;
}

static int pmic_read_temp(struct intel_pmic_opregion *opregion,
			  int reg, u64 *value)
{
	int raw_temp, temp;

	if (!opregion->data->get_raw_temp)
		return -ENXIO;

	raw_temp = opregion->data->get_raw_temp(opregion->regmap, reg);
	if (raw_temp < 0)
		return raw_temp;

	if (!opregion->lpat_table) {
		*value = raw_temp;
		return 0;
	}

	temp = acpi_lpat_raw_to_temp(opregion->lpat_table, raw_temp);
	if (temp < 0)
		return temp;

	*value = temp;
	return 0;
}

static int pmic_thermal_temp(struct intel_pmic_opregion *opregion, int reg,
			     u32 function, u64 *value)
{
	return function == ACPI_READ ?
		pmic_read_temp(opregion, reg, value) : -EINVAL;
}

static int pmic_thermal_aux(struct intel_pmic_opregion *opregion, int reg,
			    u32 function, u64 *value)
{
	int raw_temp;

	if (function == ACPI_READ)
		return pmic_read_temp(opregion, reg, value);

	if (!opregion->data->update_aux)
		return -ENXIO;

	if (opregion->lpat_table) {
		raw_temp = acpi_lpat_temp_to_raw(opregion->lpat_table, *value);
		if (raw_temp < 0)
			return raw_temp;
	} else {
		raw_temp = *value;
	}

	return opregion->data->update_aux(opregion->regmap, reg, raw_temp);
}

static int pmic_thermal_pen(struct intel_pmic_opregion *opregion, int reg,
			    u32 function, u64 *value)
{
	struct intel_pmic_opregion_data *d = opregion->data;
	struct regmap *regmap = opregion->regmap;

	if (!d->get_policy || !d->update_policy)
		return -ENXIO;

	if (function == ACPI_READ)
		return d->get_policy(regmap, reg, value);

	if (*value != 0 && *value != 1)
		return -EINVAL;

	return d->update_policy(regmap, reg, *value);
}

static bool pmic_thermal_is_temp(int address)
{
	return (address <= 0x3c) && !(address % 12);
}

static bool pmic_thermal_is_aux(int address)
{
	return (address >= 4 && address <= 0x40 && !((address - 4) % 12)) ||
	       (address >= 8 && address <= 0x44 && !((address - 8) % 12));
}

static bool pmic_thermal_is_pen(int address)
{
	return address >= 0x48 && address <= 0x5c;
}

static acpi_status intel_pmic_thermal_handler(u32 function,
		acpi_physical_address address, u32 bits, u64 *value64,
		void *handler_context, void *region_context)
{
	struct intel_pmic_opregion *opregion = region_context;
	struct intel_pmic_opregion_data *d = opregion->data;
	int reg, result;

	if (bits != 32 || !value64)
		return AE_BAD_PARAMETER;

	result = pmic_get_reg_bit(address, d->thermal_table,
				  d->thermal_table_count, &reg, NULL);
	if (result == -ENOENT)
		return AE_BAD_PARAMETER;

	mutex_lock(&opregion->lock);

	if (pmic_thermal_is_temp(address))
		result = pmic_thermal_temp(opregion, reg, function, value64);
	else if (pmic_thermal_is_aux(address))
		result = pmic_thermal_aux(opregion, reg, function, value64);
	else if (pmic_thermal_is_pen(address))
		result = pmic_thermal_pen(opregion, reg, function, value64);
	else
		result = -EINVAL;

	mutex_unlock(&opregion->lock);

	if (result < 0) {
		if (result == -EINVAL)
			return AE_BAD_PARAMETER;
		else
			return AE_ERROR;
	}

	return AE_OK;
}

int intel_pmic_install_opregion_handler(struct device *dev, acpi_handle handle,
					struct regmap *regmap,
					struct intel_pmic_opregion_data *d)
{
	acpi_status status;
	struct intel_pmic_opregion *opregion;
	int ret;

	if (!dev || !regmap || !d)
		return -EINVAL;

	if (!handle)
		return -ENODEV;

	opregion = devm_kzalloc(dev, sizeof(*opregion), GFP_KERNEL);
	if (!opregion)
		return -ENOMEM;

	mutex_init(&opregion->lock);
	opregion->regmap = regmap;
	opregion->lpat_table = acpi_lpat_get_conversion_table(handle);

	status = acpi_install_address_space_handler(handle,
						    PMIC_POWER_OPREGION_ID,
						    intel_pmic_power_handler,
						    NULL, opregion);
	if (ACPI_FAILURE(status)) {
		ret = -ENODEV;
		goto out_error;
	}

	status = acpi_install_address_space_handler(handle,
						    PMIC_THERMAL_OPREGION_ID,
						    intel_pmic_thermal_handler,
						    NULL, opregion);
	if (ACPI_FAILURE(status)) {
		acpi_remove_address_space_handler(handle, PMIC_POWER_OPREGION_ID,
						  intel_pmic_power_handler);
		ret = -ENODEV;
		goto out_error;
	}

	opregion->data = d;
	return 0;

out_error:
	acpi_lpat_free_conversion_table(opregion->lpat_table);
	return ret;
}
EXPORT_SYMBOL_GPL(intel_pmic_install_opregion_handler);

MODULE_LICENSE("GPL");
