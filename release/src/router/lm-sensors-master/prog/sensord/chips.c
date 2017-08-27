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

#include "sensord.h"

/* TODO: Temp in C/F */

/** formatters **/

static char buff[4096];

static const char *fmtExtra(int alrm, int beep)
{
	if (alrm)
		sprintf(buff + strlen(buff), " [ALARM]");
	if (beep)
		sprintf(buff + strlen(buff), " (beep)");
	return buff;
}

static const char *fmtTemps_1(const double values[], int alrm, int beep)
{
	sprintf(buff, "%.1f C (limit = %.1f C, hysteresis = %.1f C)",
		values[0], values[1], values[2]);
	return fmtExtra(alrm, beep);
}

static const char *fmtTemps_minmax_1(const double values[], int alrm,
				     int beep) {
	sprintf(buff, "%.1f C (min = %.1f C, max = %.1f C)", values[0],
		values[1], values[2]);
	return fmtExtra(alrm, beep);
}

static const char *fmtTemp_only(const double values[], int alrm, int beep)
{
	sprintf(buff, "%.1f C", values[0]);
	return fmtExtra(alrm, beep);
}

static const char *fmtVolt_2(const double values[], int alrm, int beep)
{
	sprintf(buff, "%+.2f V", values[0]);
	return fmtExtra(alrm, beep);
}

static const char *fmtVolt_3(const double values[], int alrm, int beep)
{
	sprintf(buff, "%+.3f V", values[0]);
	return fmtExtra(alrm, beep);
}

static const char *fmtVolts_2(const double values[], int alrm, int beep)
{
	sprintf(buff, "%+.2f V (min = %+.2f V, max = %+.2f V)", values[0],
		values[1], values[2]);
	return fmtExtra(alrm, beep);
}

static const char *fmtFans_0(const double values[], int alrm, int beep)
{
	sprintf(buff, "%.0f RPM (min = %.0f RPM, div = %.0f)", values[0],
		values[1], values[2]);
	return fmtExtra(alrm, beep);
}

static const char *fmtFans_nodiv_0(const double values[], int alrm, int beep)
{
	sprintf(buff, "%.0f RPM (min = %.0f RPM)", values[0], values[1]);
	return fmtExtra(alrm, beep);
}

static const char *fmtFan_only(const double values[], int alrm, int beep)
{
	sprintf(buff, "%.0f RPM", values[0]);
	return fmtExtra(alrm, beep);
}

static const char *fmtSoundAlarm(const double values[], int alrm, int beep)
{
	sprintf(buff, "Sound alarm %s",
		(values[0] < 0.5) ? "disabled" : "enabled");
	return fmtExtra(alrm, beep);
}

static const char *rrdF0(const double values[])
{
	sprintf(buff, "%.0f", values[0]);
	return buff;
}

static const char *rrdF1(const double values[])
{
	sprintf(buff, "%.1f", values[0]);
	return buff;
}

static const char *rrdF2(const double values[])
{
	sprintf(buff, "%.2f", values[0]);
	return buff;
}

static const char *rrdF3(const double values[])
{
	sprintf(buff, "%.3f", values[0]);
	return buff;
}

static void fillChipVoltage(FeatureDescriptor *voltage,
			    const sensors_chip_name *name,
			    const sensors_feature *feature)
{
	const sensors_subfeature *sf, *sfmin, *sfmax;
	int pos = 0;

	voltage->rrd = rrdF2;
	voltage->type = DataType_voltage;

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_IN_INPUT);
	if (sf)
		voltage->dataNumbers[pos++] = sf->number;

	sfmin = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_IN_MIN);
	sfmax = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_IN_MAX);
	if (sfmin && sfmax) {
		voltage->format = fmtVolts_2;
		voltage->dataNumbers[pos++] = sfmin->number;
		voltage->dataNumbers[pos++] = sfmax->number;
	} else {
		voltage->format = fmtVolt_2;
	}

	/* terminate the list */
	voltage->dataNumbers[pos] = -1;

	/* alarm if applicable */
	if ((sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_IN_ALARM)) ||
	    (sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_IN_MIN_ALARM)) ||
	    (sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_IN_MAX_ALARM))) {
		voltage->alarmNumber = sf->number;
	} else {
		voltage->alarmNumber = -1;
	}
	/* beep if applicable */
	if ((sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_IN_BEEP))) {
		voltage->beepNumber = sf->number;
	} else {
		voltage->beepNumber = -1;
	}
}

static void fillChipTemperature(FeatureDescriptor *temperature,
				const sensors_chip_name *name,
				const sensors_feature *feature)
{
	const sensors_subfeature *sf, *sfmin, *sfmax, *sfhyst;
	int pos = 0;

	temperature->rrd = rrdF1;
	temperature->type = DataType_temperature;

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_TEMP_INPUT);
	if (sf)
		temperature->dataNumbers[pos++] = sf->number;

	sfmin = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_TEMP_MIN);
	sfmax = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_TEMP_MAX);
	sfhyst = sensors_get_subfeature(name, feature,
					SENSORS_SUBFEATURE_TEMP_MAX_HYST);
	if (sfmin && sfmax) {
		temperature->format = fmtTemps_minmax_1;
		temperature->dataNumbers[pos++] = sfmin->number;
		temperature->dataNumbers[pos++] = sfmax->number;
	} else if (sfmax && sfhyst) {
		temperature->format = fmtTemps_1;
		temperature->dataNumbers[pos++] = sfmax->number;
		temperature->dataNumbers[pos++] = sfhyst->number;
	} else {
		temperature->format = fmtTemp_only;
	}

	/* terminate the list */
	temperature->dataNumbers[pos] = -1;

	/* alarm if applicable */
	if ((sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_TEMP_ALARM)) ||
	    (sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_TEMP_MAX_ALARM))) {
		temperature->alarmNumber = sf->number;
	} else {
		temperature->alarmNumber = -1;
	}
	/* beep if applicable */
	if ((sf = sensors_get_subfeature(name, feature,
					 SENSORS_SUBFEATURE_TEMP_BEEP))) {
		temperature->beepNumber = sf->number;
	} else {
		temperature->beepNumber = -1;
	}
}

static void fillChipFan(FeatureDescriptor *fan,
			const sensors_chip_name *name,
			const sensors_feature *feature)
{
	const sensors_subfeature *sf, *sfmin, *sfdiv;
	int pos = 0;

	fan->rrd = rrdF0;
	fan->type = DataType_rpm;

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_FAN_INPUT);
	if (sf)
		fan->dataNumbers[pos++] = sf->number;

	sfmin = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_MIN);
	if (sfmin) {
		fan->dataNumbers[pos++] = sfmin->number;
		sfdiv = sensors_get_subfeature(name, feature,
					       SENSORS_SUBFEATURE_FAN_DIV);
		if (sfdiv) {
			fan->format = fmtFans_0;
			fan->dataNumbers[pos++] = sfdiv->number;
		} else {
			fan->format = fmtFans_nodiv_0;
		}
	} else {
		fan->format = fmtFan_only;
	}

	/* terminate the list */
	fan->dataNumbers[pos] = -1;

	/* alarm if applicable */
	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_FAN_ALARM);
	if (sf) {
		fan->alarmNumber = sf->number;
	} else {
		fan->alarmNumber = -1;
	}
	/* beep if applicable */
	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_FAN_BEEP);
	if (sf) {
		fan->beepNumber = sf->number;
	} else {
		fan->beepNumber = -1;
	}
}

static void fillChipVid(FeatureDescriptor *vid,
			const sensors_chip_name *name,
			const sensors_feature *feature)
{
	const sensors_subfeature *sub;

	sub = sensors_get_subfeature(name, feature, SENSORS_SUBFEATURE_VID);
	if (!sub)
		return;

	vid->format = fmtVolt_3;
	vid->rrd = rrdF3;
	vid->type = DataType_voltage;
	vid->alarmNumber = -1;
	vid->beepNumber = -1;
	vid->dataNumbers[0] = sub->number;
	vid->dataNumbers[1] = -1;
}

static void fillChipBeepEnable(FeatureDescriptor *beepen,
			       const sensors_chip_name *name,
			       const sensors_feature *feature)
{
	const sensors_subfeature *sub;

	sub = sensors_get_subfeature(name, feature,
				     SENSORS_SUBFEATURE_BEEP_ENABLE);
	if (!sub)
		return;

	beepen->format = fmtSoundAlarm;
	beepen->rrd = rrdF0;
	beepen->type = DataType_other;
	beepen->alarmNumber = -1;
	beepen->beepNumber = -1;
	beepen->dataNumbers[0] = sub->number;
	beepen->dataNumbers[1] = -1;
}

static FeatureDescriptor * generateChipFeatures(const sensors_chip_name *chip)
{
	int nr, count = 1;
	const sensors_feature *sensor;
	FeatureDescriptor *features;

	/* How many main features do we have? */
	nr = 0;
	while ((sensor = sensors_get_features(chip, &nr)))
		count++;

	/* Allocate the memory we need */
	features = calloc(count, sizeof(FeatureDescriptor));
	if (!features)
		return NULL;

	/* Fill in the data structures */
	count = 0;
	nr = 0;
	while ((sensor = sensors_get_features(chip, &nr))) {
		switch (sensor->type) {
		case SENSORS_FEATURE_TEMP:
			fillChipTemperature(&features[count], chip, sensor);
			break;
		case SENSORS_FEATURE_IN:
			fillChipVoltage(&features[count], chip, sensor);
			break;
		case SENSORS_FEATURE_FAN:
			fillChipFan(&features[count], chip, sensor);
			break;
		case SENSORS_FEATURE_VID:
			fillChipVid(&features[count], chip, sensor);
			break;
		case SENSORS_FEATURE_BEEP_ENABLE:
			fillChipBeepEnable(&features[count], chip, sensor);
			break;
		default:
			continue;
		}

		features[count].feature = sensor;
		count++;
	}

	return features;
}

ChipDescriptor * knownChips;

int initKnownChips(void)
{
	int nr, count = 1;
	const sensors_chip_name *name;

	/* How many chips do we have? */
	nr = 0;
	while ((name = sensors_get_detected_chips(NULL, &nr)))
		count++;

	/* Allocate the memory we need */
	knownChips = calloc(count, sizeof(ChipDescriptor));
	if (!knownChips)
		return 1;

	/* Fill in the data structures */
	count = 0;
	nr = 0;
	while ((name = sensors_get_detected_chips(NULL, &nr))) {
		knownChips[count].name = name;
		if ((knownChips[count].features = generateChipFeatures(name)))
			count++;
	}

	return 0;
}

void freeKnownChips(void)
{
	int index0;

	for (index0 = 0; knownChips[index0].features; index0++)
		free(knownChips[index0].features);
	free(knownChips);
}
