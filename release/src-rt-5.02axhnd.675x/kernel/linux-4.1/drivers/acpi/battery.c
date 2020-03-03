/*
 *  battery.c - ACPI Battery Driver (Revision: 2.0)
 *
 *  Copyright (C) 2007 Alexey Starikovskiy <astarikovskiy@suse.de>
 *  Copyright (C) 2004-2007 Vladimir Lebedev <vladimir.p.lebedev@intel.com>
 *  Copyright (C) 2001, 2002 Andy Grover <andrew.grover@intel.com>
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/async.h>
#include <linux/dmi.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <asm/unaligned.h>

#ifdef CONFIG_ACPI_PROCFS_POWER
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#endif

#include <linux/acpi.h>
#include <linux/power_supply.h>

#include "battery.h"

#define PREFIX "ACPI: "

#define ACPI_BATTERY_VALUE_UNKNOWN 0xFFFFFFFF

#define ACPI_BATTERY_DEVICE_NAME	"Battery"

/* Battery power unit: 0 means mW, 1 means mA */
#define ACPI_BATTERY_POWER_UNIT_MA	1

#define ACPI_BATTERY_STATE_DISCHARGING	0x1
#define ACPI_BATTERY_STATE_CHARGING	0x2
#define ACPI_BATTERY_STATE_CRITICAL	0x4

#define _COMPONENT		ACPI_BATTERY_COMPONENT

ACPI_MODULE_NAME("battery");

MODULE_AUTHOR("Paul Diefenbaugh");
MODULE_AUTHOR("Alexey Starikovskiy <astarikovskiy@suse.de>");
MODULE_DESCRIPTION("ACPI Battery Driver");
MODULE_LICENSE("GPL");

static int battery_bix_broken_package;
static int battery_notification_delay_ms;
static unsigned int cache_time = 1000;
module_param(cache_time, uint, 0644);
MODULE_PARM_DESC(cache_time, "cache time in milliseconds");

#ifdef CONFIG_ACPI_PROCFS_POWER
extern struct proc_dir_entry *acpi_lock_battery_dir(void);
extern void *acpi_unlock_battery_dir(struct proc_dir_entry *acpi_battery_dir);

enum acpi_battery_files {
	info_tag = 0,
	state_tag,
	alarm_tag,
	ACPI_BATTERY_NUMFILES,
};

#endif

static const struct acpi_device_id battery_device_ids[] = {
	{"PNP0C0A", 0},
	{"", 0},
};

MODULE_DEVICE_TABLE(acpi, battery_device_ids);

enum {
	ACPI_BATTERY_ALARM_PRESENT,
	ACPI_BATTERY_XINFO_PRESENT,
	ACPI_BATTERY_QUIRK_PERCENTAGE_CAPACITY,
	/* On Lenovo Thinkpad models from 2010 and 2011, the power unit
	   switches between mWh and mAh depending on whether the system
	   is running on battery or not.  When mAh is the unit, most
	   reported values are incorrect and need to be adjusted by
	   10000/design_voltage.  Verified on x201, t410, t410s, and x220.
	   Pre-2010 and 2012 models appear to always report in mWh and
	   are thus unaffected (tested with t42, t61, t500, x200, x300,
	   and x230).  Also, in mid-2012 Lenovo issued a BIOS update for
	   the 2011 models that fixes the issue (tested on x220 with a
	   post-1.29 BIOS), but as of Nov. 2012, no such update is
	   available for the 2010 models.  */
	ACPI_BATTERY_QUIRK_THINKPAD_MAH,
};

struct acpi_battery {
	struct mutex lock;
	struct mutex sysfs_lock;
	struct power_supply *bat;
	struct power_supply_desc bat_desc;
	struct acpi_device *device;
	struct notifier_block pm_nb;
	unsigned long update_time;
	int revision;
	int rate_now;
	int capacity_now;
	int voltage_now;
	int design_capacity;
	int full_charge_capacity;
	int technology;
	int design_voltage;
	int design_capacity_warning;
	int design_capacity_low;
	int cycle_count;
	int measurement_accuracy;
	int max_sampling_time;
	int min_sampling_time;
	int max_averaging_interval;
	int min_averaging_interval;
	int capacity_granularity_1;
	int capacity_granularity_2;
	int alarm;
	char model_number[32];
	char serial_number[32];
	char type[32];
	char oem_info[32];
	int state;
	int power_unit;
	unsigned long flags;
};

#define to_acpi_battery(x) power_supply_get_drvdata(x)

static inline int acpi_battery_present(struct acpi_battery *battery)
{
	return battery->device->status.battery_present;
}

static int acpi_battery_technology(struct acpi_battery *battery)
{
	if (!strcasecmp("NiCd", battery->type))
		return POWER_SUPPLY_TECHNOLOGY_NiCd;
	if (!strcasecmp("NiMH", battery->type))
		return POWER_SUPPLY_TECHNOLOGY_NiMH;
	if (!strcasecmp("LION", battery->type))
		return POWER_SUPPLY_TECHNOLOGY_LION;
	if (!strncasecmp("LI-ION", battery->type, 6))
		return POWER_SUPPLY_TECHNOLOGY_LION;
	if (!strcasecmp("LiP", battery->type))
		return POWER_SUPPLY_TECHNOLOGY_LIPO;
	return POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
}

static int acpi_battery_get_state(struct acpi_battery *battery);

static int acpi_battery_is_charged(struct acpi_battery *battery)
{
	/* charging, discharging or critical low */
	if (battery->state != 0)
		return 0;

	/* battery not reporting charge */
	if (battery->capacity_now == ACPI_BATTERY_VALUE_UNKNOWN ||
	    battery->capacity_now == 0)
		return 0;

	/* good batteries update full_charge as the batteries degrade */
	if (battery->full_charge_capacity == battery->capacity_now)
		return 1;

	/* fallback to using design values for broken batteries */
	if (battery->design_capacity == battery->capacity_now)
		return 1;

	/* we don't do any sort of metric based on percentages */
	return 0;
}

static int acpi_battery_get_property(struct power_supply *psy,
				     enum power_supply_property psp,
				     union power_supply_propval *val)
{
	int ret = 0;
	struct acpi_battery *battery = to_acpi_battery(psy);

	if (acpi_battery_present(battery)) {
		/* run battery update only if it is present */
		acpi_battery_get_state(battery);
	} else if (psp != POWER_SUPPLY_PROP_PRESENT)
		return -ENODEV;
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (battery->state & ACPI_BATTERY_STATE_DISCHARGING)
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		else if (battery->state & ACPI_BATTERY_STATE_CHARGING)
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
		else if (acpi_battery_is_charged(battery))
			val->intval = POWER_SUPPLY_STATUS_FULL;
		else
			val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = acpi_battery_present(battery);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = acpi_battery_technology(battery);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = battery->cycle_count;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		if (battery->design_voltage == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->design_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		if (battery->voltage_now == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->voltage_now * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
	case POWER_SUPPLY_PROP_POWER_NOW:
		if (battery->rate_now == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->rate_now * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		if (battery->design_capacity == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->design_capacity * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		if (battery->full_charge_capacity == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->full_charge_capacity * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
	case POWER_SUPPLY_PROP_ENERGY_NOW:
		if (battery->capacity_now == ACPI_BATTERY_VALUE_UNKNOWN)
			ret = -ENODEV;
		else
			val->intval = battery->capacity_now * 1000;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (battery->capacity_now && battery->full_charge_capacity)
			val->intval = battery->capacity_now * 100/
					battery->full_charge_capacity;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (battery->state & ACPI_BATTERY_STATE_CRITICAL)
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
		else if (test_bit(ACPI_BATTERY_ALARM_PRESENT, &battery->flags) &&
			(battery->capacity_now <= battery->alarm))
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
		else if (acpi_battery_is_charged(battery))
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
		else
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = battery->model_number;
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = battery->oem_info;
		break;
	case POWER_SUPPLY_PROP_SERIAL_NUMBER:
		val->strval = battery->serial_number;
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static enum power_supply_property charge_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
};

static enum power_supply_property energy_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_POWER_NOW,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
	POWER_SUPPLY_PROP_ENERGY_FULL,
	POWER_SUPPLY_PROP_ENERGY_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
};

#ifdef CONFIG_ACPI_PROCFS_POWER
inline char *acpi_battery_units(struct acpi_battery *battery)
{
	return (battery->power_unit == ACPI_BATTERY_POWER_UNIT_MA) ?
		"mA" : "mW";
}
#endif

/* --------------------------------------------------------------------------
                               Battery Management
   -------------------------------------------------------------------------- */
struct acpi_offsets {
	size_t offset;		/* offset inside struct acpi_sbs_battery */
	u8 mode;		/* int or string? */
};

static struct acpi_offsets state_offsets[] = {
	{offsetof(struct acpi_battery, state), 0},
	{offsetof(struct acpi_battery, rate_now), 0},
	{offsetof(struct acpi_battery, capacity_now), 0},
	{offsetof(struct acpi_battery, voltage_now), 0},
};

static struct acpi_offsets info_offsets[] = {
	{offsetof(struct acpi_battery, power_unit), 0},
	{offsetof(struct acpi_battery, design_capacity), 0},
	{offsetof(struct acpi_battery, full_charge_capacity), 0},
	{offsetof(struct acpi_battery, technology), 0},
	{offsetof(struct acpi_battery, design_voltage), 0},
	{offsetof(struct acpi_battery, design_capacity_warning), 0},
	{offsetof(struct acpi_battery, design_capacity_low), 0},
	{offsetof(struct acpi_battery, capacity_granularity_1), 0},
	{offsetof(struct acpi_battery, capacity_granularity_2), 0},
	{offsetof(struct acpi_battery, model_number), 1},
	{offsetof(struct acpi_battery, serial_number), 1},
	{offsetof(struct acpi_battery, type), 1},
	{offsetof(struct acpi_battery, oem_info), 1},
};

static struct acpi_offsets extended_info_offsets[] = {
	{offsetof(struct acpi_battery, revision), 0},
	{offsetof(struct acpi_battery, power_unit), 0},
	{offsetof(struct acpi_battery, design_capacity), 0},
	{offsetof(struct acpi_battery, full_charge_capacity), 0},
	{offsetof(struct acpi_battery, technology), 0},
	{offsetof(struct acpi_battery, design_voltage), 0},
	{offsetof(struct acpi_battery, design_capacity_warning), 0},
	{offsetof(struct acpi_battery, design_capacity_low), 0},
	{offsetof(struct acpi_battery, cycle_count), 0},
	{offsetof(struct acpi_battery, measurement_accuracy), 0},
	{offsetof(struct acpi_battery, max_sampling_time), 0},
	{offsetof(struct acpi_battery, min_sampling_time), 0},
	{offsetof(struct acpi_battery, max_averaging_interval), 0},
	{offsetof(struct acpi_battery, min_averaging_interval), 0},
	{offsetof(struct acpi_battery, capacity_granularity_1), 0},
	{offsetof(struct acpi_battery, capacity_granularity_2), 0},
	{offsetof(struct acpi_battery, model_number), 1},
	{offsetof(struct acpi_battery, serial_number), 1},
	{offsetof(struct acpi_battery, type), 1},
	{offsetof(struct acpi_battery, oem_info), 1},
};

static int extract_package(struct acpi_battery *battery,
			   union acpi_object *package,
			   struct acpi_offsets *offsets, int num)
{
	int i;
	union acpi_object *element;
	if (package->type != ACPI_TYPE_PACKAGE)
		return -EFAULT;
	for (i = 0; i < num; ++i) {
		if (package->package.count <= i)
			return -EFAULT;
		element = &package->package.elements[i];
		if (offsets[i].mode) {
			u8 *ptr = (u8 *)battery + offsets[i].offset;
			if (element->type == ACPI_TYPE_STRING ||
			    element->type == ACPI_TYPE_BUFFER)
				strncpy(ptr, element->string.pointer, 32);
			else if (element->type == ACPI_TYPE_INTEGER) {
				strncpy(ptr, (u8 *)&element->integer.value,
					sizeof(u64));
				ptr[sizeof(u64)] = 0;
			} else
				*ptr = 0; /* don't have value */
		} else {
			int *x = (int *)((u8 *)battery + offsets[i].offset);
			*x = (element->type == ACPI_TYPE_INTEGER) ?
				element->integer.value : -1;
		}
	}
	return 0;
}

static int acpi_battery_get_status(struct acpi_battery *battery)
{
	if (acpi_bus_get_status(battery->device)) {
		ACPI_EXCEPTION((AE_INFO, AE_ERROR, "Evaluating _STA"));
		return -ENODEV;
	}
	return 0;
}

static int acpi_battery_get_info(struct acpi_battery *battery)
{
	int result = -EFAULT;
	acpi_status status = 0;
	char *name = test_bit(ACPI_BATTERY_XINFO_PRESENT, &battery->flags) ?
			"_BIX" : "_BIF";

	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	if (!acpi_battery_present(battery))
		return 0;
	mutex_lock(&battery->lock);
	status = acpi_evaluate_object(battery->device->handle, name,
						NULL, &buffer);
	mutex_unlock(&battery->lock);

	if (ACPI_FAILURE(status)) {
		ACPI_EXCEPTION((AE_INFO, status, "Evaluating %s", name));
		return -ENODEV;
	}

	if (battery_bix_broken_package)
		result = extract_package(battery, buffer.pointer,
				extended_info_offsets + 1,
				ARRAY_SIZE(extended_info_offsets) - 1);
	else if (test_bit(ACPI_BATTERY_XINFO_PRESENT, &battery->flags))
		result = extract_package(battery, buffer.pointer,
				extended_info_offsets,
				ARRAY_SIZE(extended_info_offsets));
	else
		result = extract_package(battery, buffer.pointer,
				info_offsets, ARRAY_SIZE(info_offsets));
	kfree(buffer.pointer);
	if (test_bit(ACPI_BATTERY_QUIRK_PERCENTAGE_CAPACITY, &battery->flags))
		battery->full_charge_capacity = battery->design_capacity;
	if (test_bit(ACPI_BATTERY_QUIRK_THINKPAD_MAH, &battery->flags) &&
	    battery->power_unit && battery->design_voltage) {
		battery->design_capacity = battery->design_capacity *
		    10000 / battery->design_voltage;
		battery->full_charge_capacity = battery->full_charge_capacity *
		    10000 / battery->design_voltage;
		battery->design_capacity_warning =
		    battery->design_capacity_warning *
		    10000 / battery->design_voltage;
		/* Curiously, design_capacity_low, unlike the rest of them,
		   is correct.  */
		/* capacity_granularity_* equal 1 on the systems tested, so
		   it's impossible to tell if they would need an adjustment
		   or not if their values were higher.  */
	}
	return result;
}

static int acpi_battery_get_state(struct acpi_battery *battery)
{
	int result = 0;
	acpi_status status = 0;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	if (!acpi_battery_present(battery))
		return 0;

	if (battery->update_time &&
	    time_before(jiffies, battery->update_time +
			msecs_to_jiffies(cache_time)))
		return 0;

	mutex_lock(&battery->lock);
	status = acpi_evaluate_object(battery->device->handle, "_BST",
				      NULL, &buffer);
	mutex_unlock(&battery->lock);

	if (ACPI_FAILURE(status)) {
		ACPI_EXCEPTION((AE_INFO, status, "Evaluating _BST"));
		return -ENODEV;
	}

	result = extract_package(battery, buffer.pointer,
				 state_offsets, ARRAY_SIZE(state_offsets));
	battery->update_time = jiffies;
	kfree(buffer.pointer);

	/* For buggy DSDTs that report negative 16-bit values for either
	 * charging or discharging current and/or report 0 as 65536
	 * due to bad math.
	 */
	if (battery->power_unit == ACPI_BATTERY_POWER_UNIT_MA &&
		battery->rate_now != ACPI_BATTERY_VALUE_UNKNOWN &&
		(s16)(battery->rate_now) < 0) {
		battery->rate_now = abs((s16)battery->rate_now);
		printk_once(KERN_WARNING FW_BUG
			    "battery: (dis)charge rate invalid.\n");
	}

	if (test_bit(ACPI_BATTERY_QUIRK_PERCENTAGE_CAPACITY, &battery->flags)
	    && battery->capacity_now >= 0 && battery->capacity_now <= 100)
		battery->capacity_now = (battery->capacity_now *
				battery->full_charge_capacity) / 100;
	if (test_bit(ACPI_BATTERY_QUIRK_THINKPAD_MAH, &battery->flags) &&
	    battery->power_unit && battery->design_voltage) {
		battery->capacity_now = battery->capacity_now *
		    10000 / battery->design_voltage;
	}
	return result;
}

static int acpi_battery_set_alarm(struct acpi_battery *battery)
{
	acpi_status status = 0;

	if (!acpi_battery_present(battery) ||
	    !test_bit(ACPI_BATTERY_ALARM_PRESENT, &battery->flags))
		return -ENODEV;

	mutex_lock(&battery->lock);
	status = acpi_execute_simple_method(battery->device->handle, "_BTP",
					    battery->alarm);
	mutex_unlock(&battery->lock);

	if (ACPI_FAILURE(status))
		return -ENODEV;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Alarm set to %d\n", battery->alarm));
	return 0;
}

static int acpi_battery_init_alarm(struct acpi_battery *battery)
{
	/* See if alarms are supported, and if so, set default */
	if (!acpi_has_method(battery->device->handle, "_BTP")) {
		clear_bit(ACPI_BATTERY_ALARM_PRESENT, &battery->flags);
		return 0;
	}
	set_bit(ACPI_BATTERY_ALARM_PRESENT, &battery->flags);
	if (!battery->alarm)
		battery->alarm = battery->design_capacity_warning;
	return acpi_battery_set_alarm(battery);
}

static ssize_t acpi_battery_alarm_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct acpi_battery *battery = to_acpi_battery(dev_get_drvdata(dev));
	return sprintf(buf, "%d\n", battery->alarm * 1000);
}

static ssize_t acpi_battery_alarm_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long x;
	struct acpi_battery *battery = to_acpi_battery(dev_get_drvdata(dev));
	if (sscanf(buf, "%lu\n", &x) == 1)
		battery->alarm = x/1000;
	if (acpi_battery_present(battery))
		acpi_battery_set_alarm(battery);
	return count;
}

static struct device_attribute alarm_attr = {
	.attr = {.name = "alarm", .mode = 0644},
	.show = acpi_battery_alarm_show,
	.store = acpi_battery_alarm_store,
};

static int sysfs_add_battery(struct acpi_battery *battery)
{
	struct power_supply_config psy_cfg = { .drv_data = battery, };

	if (battery->power_unit == ACPI_BATTERY_POWER_UNIT_MA) {
		battery->bat_desc.properties = charge_battery_props;
		battery->bat_desc.num_properties =
			ARRAY_SIZE(charge_battery_props);
	} else {
		battery->bat_desc.properties = energy_battery_props;
		battery->bat_desc.num_properties =
			ARRAY_SIZE(energy_battery_props);
	}

	battery->bat_desc.name = acpi_device_bid(battery->device);
	battery->bat_desc.type = POWER_SUPPLY_TYPE_BATTERY;
	battery->bat_desc.get_property = acpi_battery_get_property;

	battery->bat = power_supply_register_no_ws(&battery->device->dev,
				&battery->bat_desc, &psy_cfg);

	if (IS_ERR(battery->bat)) {
		int result = PTR_ERR(battery->bat);

		battery->bat = NULL;
		return result;
	}
	return device_create_file(&battery->bat->dev, &alarm_attr);
}

static void sysfs_remove_battery(struct acpi_battery *battery)
{
	mutex_lock(&battery->sysfs_lock);
	if (!battery->bat) {
		mutex_unlock(&battery->sysfs_lock);
		return;
	}

	device_remove_file(&battery->bat->dev, &alarm_attr);
	power_supply_unregister(battery->bat);
	battery->bat = NULL;
	mutex_unlock(&battery->sysfs_lock);
}

static void find_battery(const struct dmi_header *dm, void *private)
{
	struct acpi_battery *battery = (struct acpi_battery *)private;
	/* Note: the hardcoded offsets below have been extracted from
	   the source code of dmidecode.  */
	if (dm->type == DMI_ENTRY_PORTABLE_BATTERY && dm->length >= 8) {
		const u8 *dmi_data = (const u8 *)(dm + 1);
		int dmi_capacity = get_unaligned((const u16 *)(dmi_data + 6));
		if (dm->length >= 18)
			dmi_capacity *= dmi_data[17];
		if (battery->design_capacity * battery->design_voltage / 1000
		    != dmi_capacity &&
		    battery->design_capacity * 10 == dmi_capacity)
			set_bit(ACPI_BATTERY_QUIRK_THINKPAD_MAH,
				&battery->flags);
	}
}

/*
 * According to the ACPI spec, some kinds of primary batteries can
 * report percentage battery remaining capacity directly to OS.
 * In this case, it reports the Last Full Charged Capacity == 100
 * and BatteryPresentRate == 0xFFFFFFFF.
 *
 * Now we found some battery reports percentage remaining capacity
 * even if it's rechargeable.
 * https://bugzilla.kernel.org/show_bug.cgi?id=15979
 *
 * Handle this correctly so that they won't break userspace.
 */
static void acpi_battery_quirks(struct acpi_battery *battery)
{
	if (test_bit(ACPI_BATTERY_QUIRK_PERCENTAGE_CAPACITY, &battery->flags))
		return;

	if (battery->full_charge_capacity == 100 &&
		battery->rate_now == ACPI_BATTERY_VALUE_UNKNOWN &&
		battery->capacity_now >= 0 && battery->capacity_now <= 100) {
		set_bit(ACPI_BATTERY_QUIRK_PERCENTAGE_CAPACITY, &battery->flags);
		battery->full_charge_capacity = battery->design_capacity;
		battery->capacity_now = (battery->capacity_now *
				battery->full_charge_capacity) / 100;
	}

	if (test_bit(ACPI_BATTERY_QUIRK_THINKPAD_MAH, &battery->flags))
		return;

	if (battery->power_unit && dmi_name_in_vendors("LENOVO")) {
		const char *s;
		s = dmi_get_system_info(DMI_PRODUCT_VERSION);
		if (s && !strncasecmp(s, "ThinkPad", 8)) {
			dmi_walk(find_battery, battery);
			if (test_bit(ACPI_BATTERY_QUIRK_THINKPAD_MAH,
				     &battery->flags) &&
			    battery->design_voltage) {
				battery->design_capacity =
				    battery->design_capacity *
				    10000 / battery->design_voltage;
				battery->full_charge_capacity =
				    battery->full_charge_capacity *
				    10000 / battery->design_voltage;
				battery->design_capacity_warning =
				    battery->design_capacity_warning *
				    10000 / battery->design_voltage;
				battery->capacity_now = battery->capacity_now *
				    10000 / battery->design_voltage;
			}
		}
	}
}

static int acpi_battery_update(struct acpi_battery *battery, bool resume)
{
	int result, old_present = acpi_battery_present(battery);
	result = acpi_battery_get_status(battery);
	if (result)
		return result;
	if (!acpi_battery_present(battery)) {
		sysfs_remove_battery(battery);
		battery->update_time = 0;
		return 0;
	}

	if (resume)
		return 0;

	if (!battery->update_time ||
	    old_present != acpi_battery_present(battery)) {
		result = acpi_battery_get_info(battery);
		if (result)
			return result;
		acpi_battery_init_alarm(battery);
	}
	if (!battery->bat) {
		result = sysfs_add_battery(battery);
		if (result)
			return result;
	}
	result = acpi_battery_get_state(battery);
	if (result)
		return result;
	acpi_battery_quirks(battery);

	/*
	 * Wakeup the system if battery is critical low
	 * or lower than the alarm level
	 */
	if ((battery->state & ACPI_BATTERY_STATE_CRITICAL) ||
	    (test_bit(ACPI_BATTERY_ALARM_PRESENT, &battery->flags) &&
            (battery->capacity_now <= battery->alarm)))
		pm_wakeup_event(&battery->device->dev, 0);

	return result;
}

static void acpi_battery_refresh(struct acpi_battery *battery)
{
	int power_unit;

	if (!battery->bat)
		return;

	power_unit = battery->power_unit;

	acpi_battery_get_info(battery);

	if (power_unit == battery->power_unit)
		return;

	/* The battery has changed its reporting units. */
	sysfs_remove_battery(battery);
	sysfs_add_battery(battery);
}

/* --------------------------------------------------------------------------
                              FS Interface (/proc)
   -------------------------------------------------------------------------- */

#ifdef CONFIG_ACPI_PROCFS_POWER
static struct proc_dir_entry *acpi_battery_dir;

static int acpi_battery_print_info(struct seq_file *seq, int result)
{
	struct acpi_battery *battery = seq->private;

	if (result)
		goto end;

	seq_printf(seq, "present:                 %s\n",
		   acpi_battery_present(battery) ? "yes" : "no");
	if (!acpi_battery_present(battery))
		goto end;
	if (battery->design_capacity == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "design capacity:         unknown\n");
	else
		seq_printf(seq, "design capacity:         %d %sh\n",
			   battery->design_capacity,
			   acpi_battery_units(battery));

	if (battery->full_charge_capacity == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "last full capacity:      unknown\n");
	else
		seq_printf(seq, "last full capacity:      %d %sh\n",
			   battery->full_charge_capacity,
			   acpi_battery_units(battery));

	seq_printf(seq, "battery technology:      %srechargeable\n",
		   (!battery->technology)?"non-":"");

	if (battery->design_voltage == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "design voltage:          unknown\n");
	else
		seq_printf(seq, "design voltage:          %d mV\n",
			   battery->design_voltage);
	seq_printf(seq, "design capacity warning: %d %sh\n",
		   battery->design_capacity_warning,
		   acpi_battery_units(battery));
	seq_printf(seq, "design capacity low:     %d %sh\n",
		   battery->design_capacity_low,
		   acpi_battery_units(battery));
	seq_printf(seq, "cycle count:		  %i\n", battery->cycle_count);
	seq_printf(seq, "capacity granularity 1:  %d %sh\n",
		   battery->capacity_granularity_1,
		   acpi_battery_units(battery));
	seq_printf(seq, "capacity granularity 2:  %d %sh\n",
		   battery->capacity_granularity_2,
		   acpi_battery_units(battery));
	seq_printf(seq, "model number:            %s\n", battery->model_number);
	seq_printf(seq, "serial number:           %s\n", battery->serial_number);
	seq_printf(seq, "battery type:            %s\n", battery->type);
	seq_printf(seq, "OEM info:                %s\n", battery->oem_info);
      end:
	if (result)
		seq_printf(seq, "ERROR: Unable to read battery info\n");
	return result;
}

static int acpi_battery_print_state(struct seq_file *seq, int result)
{
	struct acpi_battery *battery = seq->private;

	if (result)
		goto end;

	seq_printf(seq, "present:                 %s\n",
		   acpi_battery_present(battery) ? "yes" : "no");
	if (!acpi_battery_present(battery))
		goto end;

	seq_printf(seq, "capacity state:          %s\n",
			(battery->state & 0x04) ? "critical" : "ok");
	if ((battery->state & 0x01) && (battery->state & 0x02))
		seq_printf(seq,
			   "charging state:          charging/discharging\n");
	else if (battery->state & 0x01)
		seq_printf(seq, "charging state:          discharging\n");
	else if (battery->state & 0x02)
		seq_printf(seq, "charging state:          charging\n");
	else
		seq_printf(seq, "charging state:          charged\n");

	if (battery->rate_now == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "present rate:            unknown\n");
	else
		seq_printf(seq, "present rate:            %d %s\n",
			   battery->rate_now, acpi_battery_units(battery));

	if (battery->capacity_now == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "remaining capacity:      unknown\n");
	else
		seq_printf(seq, "remaining capacity:      %d %sh\n",
			   battery->capacity_now, acpi_battery_units(battery));
	if (battery->voltage_now == ACPI_BATTERY_VALUE_UNKNOWN)
		seq_printf(seq, "present voltage:         unknown\n");
	else
		seq_printf(seq, "present voltage:         %d mV\n",
			   battery->voltage_now);
      end:
	if (result)
		seq_printf(seq, "ERROR: Unable to read battery state\n");

	return result;
}

static int acpi_battery_print_alarm(struct seq_file *seq, int result)
{
	struct acpi_battery *battery = seq->private;

	if (result)
		goto end;

	if (!acpi_battery_present(battery)) {
		seq_printf(seq, "present:                 no\n");
		goto end;
	}
	seq_printf(seq, "alarm:                   ");
	if (!battery->alarm)
		seq_printf(seq, "unsupported\n");
	else
		seq_printf(seq, "%u %sh\n", battery->alarm,
				acpi_battery_units(battery));
      end:
	if (result)
		seq_printf(seq, "ERROR: Unable to read battery alarm\n");
	return result;
}

static ssize_t acpi_battery_write_alarm(struct file *file,
					const char __user * buffer,
					size_t count, loff_t * ppos)
{
	int result = 0;
	char alarm_string[12] = { '\0' };
	struct seq_file *m = file->private_data;
	struct acpi_battery *battery = m->private;

	if (!battery || (count > sizeof(alarm_string) - 1))
		return -EINVAL;
	if (!acpi_battery_present(battery)) {
		result = -ENODEV;
		goto end;
	}
	if (copy_from_user(alarm_string, buffer, count)) {
		result = -EFAULT;
		goto end;
	}
	alarm_string[count] = '\0';
	if (kstrtoint(alarm_string, 0, &battery->alarm)) {
		result = -EINVAL;
		goto end;
	}
	result = acpi_battery_set_alarm(battery);
      end:
	if (!result)
		return count;
	return result;
}

typedef int(*print_func)(struct seq_file *seq, int result);

static print_func acpi_print_funcs[ACPI_BATTERY_NUMFILES] = {
	acpi_battery_print_info,
	acpi_battery_print_state,
	acpi_battery_print_alarm,
};

static int acpi_battery_read(int fid, struct seq_file *seq)
{
	struct acpi_battery *battery = seq->private;
	int result = acpi_battery_update(battery, false);
	return acpi_print_funcs[fid](seq, result);
}

#define DECLARE_FILE_FUNCTIONS(_name) \
static int acpi_battery_read_##_name(struct seq_file *seq, void *offset) \
{ \
	return acpi_battery_read(_name##_tag, seq); \
} \
static int acpi_battery_##_name##_open_fs(struct inode *inode, struct file *file) \
{ \
	return single_open(file, acpi_battery_read_##_name, PDE_DATA(inode)); \
}

DECLARE_FILE_FUNCTIONS(info);
DECLARE_FILE_FUNCTIONS(state);
DECLARE_FILE_FUNCTIONS(alarm);

#undef DECLARE_FILE_FUNCTIONS

#define FILE_DESCRIPTION_RO(_name) \
	{ \
	.name = __stringify(_name), \
	.mode = S_IRUGO, \
	.ops = { \
		.open = acpi_battery_##_name##_open_fs, \
		.read = seq_read, \
		.llseek = seq_lseek, \
		.release = single_release, \
		.owner = THIS_MODULE, \
		}, \
	}

#define FILE_DESCRIPTION_RW(_name) \
	{ \
	.name = __stringify(_name), \
	.mode = S_IFREG | S_IRUGO | S_IWUSR, \
	.ops = { \
		.open = acpi_battery_##_name##_open_fs, \
		.read = seq_read, \
		.llseek = seq_lseek, \
		.write = acpi_battery_write_##_name, \
		.release = single_release, \
		.owner = THIS_MODULE, \
		}, \
	}

static const struct battery_file {
	struct file_operations ops;
	umode_t mode;
	const char *name;
} acpi_battery_file[] = {
	FILE_DESCRIPTION_RO(info),
	FILE_DESCRIPTION_RO(state),
	FILE_DESCRIPTION_RW(alarm),
};

#undef FILE_DESCRIPTION_RO
#undef FILE_DESCRIPTION_RW

static int acpi_battery_add_fs(struct acpi_device *device)
{
	struct proc_dir_entry *entry = NULL;
	int i;

	printk(KERN_WARNING PREFIX "Deprecated procfs I/F for battery is loaded,"
			" please retry with CONFIG_ACPI_PROCFS_POWER cleared\n");
	if (!acpi_device_dir(device)) {
		acpi_device_dir(device) = proc_mkdir(acpi_device_bid(device),
						     acpi_battery_dir);
		if (!acpi_device_dir(device))
			return -ENODEV;
	}

	for (i = 0; i < ACPI_BATTERY_NUMFILES; ++i) {
		entry = proc_create_data(acpi_battery_file[i].name,
					 acpi_battery_file[i].mode,
					 acpi_device_dir(device),
					 &acpi_battery_file[i].ops,
					 acpi_driver_data(device));
		if (!entry)
			return -ENODEV;
	}
	return 0;
}

static void acpi_battery_remove_fs(struct acpi_device *device)
{
	int i;
	if (!acpi_device_dir(device))
		return;
	for (i = 0; i < ACPI_BATTERY_NUMFILES; ++i)
		remove_proc_entry(acpi_battery_file[i].name,
				  acpi_device_dir(device));

	remove_proc_entry(acpi_device_bid(device), acpi_battery_dir);
	acpi_device_dir(device) = NULL;
}

#endif

/* --------------------------------------------------------------------------
                                 Driver Interface
   -------------------------------------------------------------------------- */

static void acpi_battery_notify(struct acpi_device *device, u32 event)
{
	struct acpi_battery *battery = acpi_driver_data(device);
	struct power_supply *old;

	if (!battery)
		return;
	old = battery->bat;
	/*
	* On Acer Aspire V5-573G notifications are sometimes triggered too
	* early. For example, when AC is unplugged and notification is
	* triggered, battery state is still reported as "Full", and changes to
	* "Discharging" only after short delay, without any notification.
	*/
	if (battery_notification_delay_ms > 0)
		msleep(battery_notification_delay_ms);
	if (event == ACPI_BATTERY_NOTIFY_INFO)
		acpi_battery_refresh(battery);
	acpi_battery_update(battery, false);
	acpi_bus_generate_netlink_event(device->pnp.device_class,
					dev_name(&device->dev), event,
					acpi_battery_present(battery));
	acpi_notifier_call_chain(device, event, acpi_battery_present(battery));
	/* acpi_battery_update could remove power_supply object */
	if (old && battery->bat)
		power_supply_changed(battery->bat);
}

static int battery_notify(struct notifier_block *nb,
			       unsigned long mode, void *_unused)
{
	struct acpi_battery *battery = container_of(nb, struct acpi_battery,
						    pm_nb);
	int result;

	switch (mode) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		if (!acpi_battery_present(battery))
			return 0;

		if (!battery->bat) {
			result = acpi_battery_get_info(battery);
			if (result)
				return result;

			result = sysfs_add_battery(battery);
			if (result)
				return result;
		} else
			acpi_battery_refresh(battery);

		acpi_battery_init_alarm(battery);
		acpi_battery_get_state(battery);
		break;
	}

	return 0;
}

static int battery_bix_broken_package_quirk(const struct dmi_system_id *d)
{
	battery_bix_broken_package = 1;
	return 0;
}

static int battery_notification_delay_quirk(const struct dmi_system_id *d)
{
	battery_notification_delay_ms = 1000;
	return 0;
}

static struct dmi_system_id bat_dmi_table[] = {
	{
		.callback = battery_bix_broken_package_quirk,
		.ident = "NEC LZ750/LS",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "NEC"),
			DMI_MATCH(DMI_PRODUCT_NAME, "PC-LZ750LS"),
		},
	},
	{
		.callback = battery_notification_delay_quirk,
		.ident = "Acer Aspire V5-573G",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
			DMI_MATCH(DMI_PRODUCT_NAME, "Aspire V5-573G"),
		},
	},
	{},
};

/*
 * Some machines'(E,G Lenovo Z480) ECs are not stable
 * during boot up and this causes battery driver fails to be
 * probed due to failure of getting battery information
 * from EC sometimes. After several retries, the operation
 * may work. So add retry code here and 20ms sleep between
 * every retries.
 */
static int acpi_battery_update_retry(struct acpi_battery *battery)
{
	int retry, ret;

	for (retry = 5; retry; retry--) {
		ret = acpi_battery_update(battery, false);
		if (!ret)
			break;

		msleep(20);
	}
	return ret;
}

static int acpi_battery_add(struct acpi_device *device)
{
	int result = 0;
	struct acpi_battery *battery = NULL;

	if (!device)
		return -EINVAL;

	if (device->dep_unmet)
		return -EPROBE_DEFER;

	battery = kzalloc(sizeof(struct acpi_battery), GFP_KERNEL);
	if (!battery)
		return -ENOMEM;
	battery->device = device;
	strcpy(acpi_device_name(device), ACPI_BATTERY_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_BATTERY_CLASS);
	device->driver_data = battery;
	mutex_init(&battery->lock);
	mutex_init(&battery->sysfs_lock);
	if (acpi_has_method(battery->device->handle, "_BIX"))
		set_bit(ACPI_BATTERY_XINFO_PRESENT, &battery->flags);

	result = acpi_battery_update_retry(battery);
	if (result)
		goto fail;

#ifdef CONFIG_ACPI_PROCFS_POWER
	result = acpi_battery_add_fs(device);
#endif
	if (result) {
#ifdef CONFIG_ACPI_PROCFS_POWER
		acpi_battery_remove_fs(device);
#endif
		goto fail;
	}

	printk(KERN_INFO PREFIX "%s Slot [%s] (battery %s)\n",
		ACPI_BATTERY_DEVICE_NAME, acpi_device_bid(device),
		device->status.battery_present ? "present" : "absent");

	battery->pm_nb.notifier_call = battery_notify;
	register_pm_notifier(&battery->pm_nb);

	device_init_wakeup(&device->dev, 1);

	return result;

fail:
	sysfs_remove_battery(battery);
	mutex_destroy(&battery->lock);
	mutex_destroy(&battery->sysfs_lock);
	kfree(battery);
	return result;
}

static int acpi_battery_remove(struct acpi_device *device)
{
	struct acpi_battery *battery = NULL;

	if (!device || !acpi_driver_data(device))
		return -EINVAL;
	device_init_wakeup(&device->dev, 0);
	battery = acpi_driver_data(device);
	unregister_pm_notifier(&battery->pm_nb);
#ifdef CONFIG_ACPI_PROCFS_POWER
	acpi_battery_remove_fs(device);
#endif
	sysfs_remove_battery(battery);
	mutex_destroy(&battery->lock);
	mutex_destroy(&battery->sysfs_lock);
	kfree(battery);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
/* this is needed to learn about changes made in suspended state */
static int acpi_battery_resume(struct device *dev)
{
	struct acpi_battery *battery;

	if (!dev)
		return -EINVAL;

	battery = acpi_driver_data(to_acpi_device(dev));
	if (!battery)
		return -EINVAL;

	battery->update_time = 0;
	acpi_battery_update(battery, true);
	return 0;
}
#else
#define acpi_battery_resume NULL
#endif

static SIMPLE_DEV_PM_OPS(acpi_battery_pm, NULL, acpi_battery_resume);

static struct acpi_driver acpi_battery_driver = {
	.name = "battery",
	.class = ACPI_BATTERY_CLASS,
	.ids = battery_device_ids,
	.flags = ACPI_DRIVER_ALL_NOTIFY_EVENTS,
	.ops = {
		.add = acpi_battery_add,
		.remove = acpi_battery_remove,
		.notify = acpi_battery_notify,
		},
	.drv.pm = &acpi_battery_pm,
};

static void __init acpi_battery_init_async(void *unused, async_cookie_t cookie)
{
	if (acpi_disabled)
		return;

	dmi_check_system(bat_dmi_table);
	
#ifdef CONFIG_ACPI_PROCFS_POWER
	acpi_battery_dir = acpi_lock_battery_dir();
	if (!acpi_battery_dir)
		return;
#endif
	if (acpi_bus_register_driver(&acpi_battery_driver) < 0) {
#ifdef CONFIG_ACPI_PROCFS_POWER
		acpi_unlock_battery_dir(acpi_battery_dir);
#endif
		return;
	}
	return;
}

static int __init acpi_battery_init(void)
{
	async_schedule(acpi_battery_init_async, NULL);
	return 0;
}

static void __exit acpi_battery_exit(void)
{
	acpi_bus_unregister_driver(&acpi_battery_driver);
#ifdef CONFIG_ACPI_PROCFS_POWER
	acpi_unlock_battery_dir(acpi_battery_dir);
#endif
}

module_init(acpi_battery_init);
module_exit(acpi_battery_exit);
