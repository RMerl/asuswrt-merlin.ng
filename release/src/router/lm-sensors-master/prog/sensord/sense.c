/*
 * sensord
 *
 * A daemon that periodically logs sensor information to syslog.
 *
 * Copyright (c) 1999-2002 Merlin Hughes <merlin@merlin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "args.h"
#include "sensord.h"
#include "lib/error.h"

#define DO_READ 0
#define DO_SCAN 1
#define DO_SET 2
#define DO_RRD 3

static const char *chipName(const sensors_chip_name *chip)
{
	static char buffer[256];
	if (sensors_snprintf_chip_name(buffer, 256, chip) < 0)
		return NULL;
	return buffer;
}

static int idChip(const sensors_chip_name *chip)
{
	const char *name, *adapter;

	name = chipName(chip);
	if (!name) {
		sensorLog(LOG_ERR, "Error getting chip name");
		return -1;
	}

	sensorLog(LOG_INFO, "Chip: %s", name);

	adapter = sensors_get_adapter_name(&chip->bus);
	if (!adapter)
		sensorLog(LOG_INFO, "Error getting adapter name");
	else
		sensorLog(LOG_INFO, "Adapter: %s", adapter);

	return 0;
}

static int get_flag(const sensors_chip_name *chip, int num)
{
	double val;
	int ret;

	if (num == -1)
		return 0;

	ret = sensors_get_value(chip, num, &val);
	if (ret) {
		sensorLog(LOG_ERR, "Error getting sensor data: %s/#%d: %s",
			  chip->prefix, num, sensors_strerror(ret));
		return -1;
	}

	return (int) (val + 0.5);
}

static int do_features(const sensors_chip_name *chip,
		       const FeatureDescriptor *feature, int action)
{
	char *label;
	const char *formatted;
	int i, alrm, beep, ret;
	double val[MAX_DATA];

	/* If only scanning, take a quick exit if alarm is off */
	alrm = get_flag(chip, feature->alarmNumber);
	if (alrm == -1)
		return -1;
	if (action == DO_SCAN && !alrm)
		return 0;

	for (i = 0; feature->dataNumbers[i] >= 0; i++) {
		ret = sensors_get_value(chip, feature->dataNumbers[i],
					val + i);
		if (ret) {
			sensorLog(LOG_ERR,
				  "Error getting sensor data: %s/#%d: %s",
				  chip->prefix, feature->dataNumbers[i],
				  sensors_strerror(ret));
			return -1;
		}
	}

	/* For RRD, we don't need anything else */
	if (action == DO_RRD) {
		if (feature->rrd) {
			const char *rrded = feature->rrd(val);

			sprintf(rrdBuff + strlen(rrdBuff), ":%s",
				rrded ? rrded : "U");
		}

		return 0;
	}

	/* For scanning and logging, we need extra information */
	beep = get_flag(chip, feature->beepNumber);
	if (beep == -1)
		return -1;

	formatted = feature->format(val, alrm, beep);
	if (!formatted) {
		sensorLog(LOG_ERR, "Error formatting sensor data");
		return -1;
	}

	/* FIXME: It would be more efficient to store the label at
	 * initialization time.
	 */
	label = sensors_get_label(chip, feature->feature);
	if (!label) {
		sensorLog(LOG_ERR, "Error getting sensor label: %s/%s",
			  chip->prefix, feature->feature->name);
		return -1;
	}

	if (action == DO_READ)
		sensorLog(LOG_INFO, "  %s: %s", label, formatted);
	else
		sensorLog(LOG_ALERT, "Sensor alarm: Chip %s: %s: %s",
			  chipName(chip), label, formatted);

	free(label);

	return 0;
}

static int doKnownChip(const sensors_chip_name *chip,
		       const ChipDescriptor *descriptor, int action)
{
	const FeatureDescriptor *features = descriptor->features;
	int i, ret = 0;

	if (action == DO_READ) {
		ret = idChip(chip);
		if (ret)
			return ret;
	}

	for (i = 0; features[i].format; i++) {
		ret = do_features(chip, features + i, action);
		if (ret == -1)
			break;
	}

	return ret;
}

static int setChip(const sensors_chip_name *chip)
{
	int ret = 0;
	if ((ret = idChip(chip))) {
		sensorLog(LOG_ERR, "Error identifying chip: %s",
			  chip->prefix);
	} else if ((ret = sensors_do_chip_sets(chip))) {
		sensorLog(LOG_ERR, "Error performing chip sets: %s: %s",
			  chip->prefix, sensors_strerror(ret));
		ret = 50;
	} else {
		sensorLog(LOG_INFO, "Set.");
	}
	return ret;
}

static int doChip(const sensors_chip_name *chip, int action)
{
	int ret = 0;
	if (action == DO_SET) {
		ret = setChip(chip);
	} else {
		int index0, chipindex = -1;
		for (index0 = 0; knownChips[index0].features; ++index0) {
			/*
			 * Trick: we compare addresses here. We know it works
			 * because both pointers were returned by
			 * sensors_get_detected_chips(), so they refer to
			 * libsensors internal structures, which do not move.
			 */
			if (knownChips[index0].name == chip) {
				chipindex = index0;
				break;
			}
		}

		if (chipindex >= 0) {
			ret = doKnownChip(chip, &knownChips[chipindex],
					  action);
		}
	}
	return ret;
}

static int doChips(int action)
{
	const sensors_chip_name *chip, *chip_arg;
	int i, j, ret = 0;

	for (j = 0; j < sensord_args.numChipNames; j++) {
		chip_arg = &sensord_args.chipNames[j];
		i = 0;
		while ((chip = sensors_get_detected_chips(chip_arg, &i))) {
			ret = doChip(chip, action);
			if (ret)
				return ret;
		}
	}
	return ret;
}

int readChips(void)
{
	int ret = 0;

	sensorLog(LOG_DEBUG, "sensor read started");
	ret = doChips(DO_READ);
	sensorLog(LOG_DEBUG, "sensor read finished");

	return ret;
}

int scanChips(void)
{
	int ret = 0;

	sensorLog(LOG_DEBUG, "sensor sweep started");
	ret = doChips(DO_SCAN);
	sensorLog(LOG_DEBUG, "sensor sweep finished");

	return ret;
}

int setChips(void)
{
	int ret = 0;

	sensorLog(LOG_DEBUG, "sensor set started");
	ret = doChips(DO_SET);
	sensorLog(LOG_DEBUG, "sensor set finished");

	return ret;
}

/* TODO: loadavg entry */

int rrdChips(void)
{
	int ret = 0;

	strcpy(rrdBuff, "N");

	sensorLog(LOG_DEBUG, "sensor rrd started");
	ret = doChips(DO_RRD);
	sensorLog(LOG_DEBUG, "sensor rrd finished");

	return ret;
}
