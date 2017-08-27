/*
    chips.c - Part of sensors, a user-space program for hardware monitoring
    Copyright (C) 1998-2003  Frodo Looijaard <frodol@dds.nl> and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2007-2012  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "chips.h"
#include "lib/sensors.h"
#include "lib/error.h"

#define ARRAY_SIZE(arr) (int)(sizeof(arr) / sizeof((arr)[0]))

void print_chip_raw(const sensors_chip_name *name)
{
	int a, b, err;
	const sensors_feature *feature;
	const sensors_subfeature *sub;
	char *label;
	double val;

	a = 0;
	while ((feature = sensors_get_features(name, &a))) {
		if (!(label = sensors_get_label(name, feature))) {
			fprintf(stderr, "ERROR: Can't get label of feature "
				"%s!\n", feature->name);
			continue;
		}
		printf("%s:\n", label);
		free(label);

		b = 0;
		while ((sub = sensors_get_all_subfeatures(name, feature, &b))) {
			if (sub->flags & SENSORS_MODE_R) {
				if ((err = sensors_get_value(name, sub->number,
							     &val)))
					fprintf(stderr, "ERROR: Can't get "
						"value of subfeature %s: %s\n",
						sub->name,
						sensors_strerror(err));
				else
					printf("  %s: %.3f\n", sub->name, val);
			} else
				printf("(%s)\n", label);
		}
	}
}

static const char hyst_str[] = "hyst";

static inline double deg_ctof(double cel)
{
	return cel * (9.0F / 5.0F) + 32.0F;
}

static void print_label(const char *label, int space)
{
	int len = strlen(label)+1;
	printf("%s:%*s", label, space - len, "");
}

static double get_value(const sensors_chip_name *name,
			const sensors_subfeature *sub)
{
	double val;
	int err;

	err = sensors_get_value(name, sub->number, &val);
	if (err) {
		fprintf(stderr, "ERROR: Can't get value of subfeature %s: %s\n",
			sub->name, sensors_strerror(err));
		val = 0;
	}
	return val;
}

/* A variant for input values, where we want to handle errors gracefully */
static int get_input_value(const sensors_chip_name *name,
			   const sensors_subfeature *sub,
			   double *val)
{
	int err;

	err = sensors_get_value(name, sub->number, val);
	if (err && err != -SENSORS_ERR_ACCESS_R) {
		fprintf(stderr, "ERROR: Can't get value of subfeature %s: %s\n",
			sub->name, sensors_strerror(err));
	}
	return err;
}

static int get_label_size(const sensors_chip_name *name)
{
	int i;
	const sensors_feature *iter;
	char *label;
	unsigned int max_size = 11;	/* 11 as minimum label width */

	i = 0;
	while ((iter = sensors_get_features(name, &i))) {
		if ((label = sensors_get_label(name, iter)) &&
		    strlen(label) > max_size)
			max_size = strlen(label);
		free(label);
	}

	/* One more for the colon, and one more to guarantee at least one
	   space between that colon and the value */
	return max_size + 2;
}

static void print_alarms(struct sensor_subfeature_data *alarms, int alarm_count,
			 int leading_spaces)
{
	int i, printed;

	printf("%*s", leading_spaces + 7, "ALARM");
	if (alarm_count > 1 || alarms[0].name) {
		printf(" (");
		for (i = printed = 0; i < alarm_count; i++) {
			if (alarms[i].name) {
				if (printed)
					printf(", ");
				printf("%s", alarms[i].name);
				printed = 1;
			}
		}
		printf(")");
	}
}

static void print_limits(struct sensor_subfeature_data *limits,
			 int limit_count,
			 struct sensor_subfeature_data *alarms,
			 int alarm_count, int label_size,
			 const char *fmt)
{
	int i, slot, skip;
	int alarms_printed = 0;

	/*
	 * We print limits on two columns, filling lines first, except for
	 * hysteresis which must always go on the right column, with the
	 * limit it relates to being in the left column on the same line.
	 */
	for (i = slot = 0; i < limit_count; i++, slot++) {
		if (!(slot & 1)) {
			if (slot)
				printf("\n%*s", label_size + 10, "");
			printf("(");
		} else {
			printf(", ");
		}
		printf(fmt, limits[i].name, limits[i].value,
			     limits[i].unit);

		/* If needed, skip one slot to avoid hyst on first column */
		skip = i + 2 < limit_count && limits[i + 2].name == hyst_str &&
		       !(slot & 1);

		if (((slot + skip) & 1) || i == limit_count - 1) {
			printf(")");
			if (alarm_count && !alarms_printed) {
				print_alarms(alarms, alarm_count,
					     (slot & 1) ? 0 : 16);
				alarms_printed = 1;
			}
		}
		slot += skip;
	}
	if (alarm_count && !alarms_printed)
		print_alarms(alarms, alarm_count, 32);
}

/*
 * Get sensor limit information.
 * *num_limits and *num_alarms must be initialized by the caller.
 */
static void get_sensor_limit_data(const sensors_chip_name *name,
				  const sensors_feature *feature,
				  const struct sensor_subfeature_list *sfl,
				  struct sensor_subfeature_data *limits,
				  int *num_limits,
				  struct sensor_subfeature_data *alarms,
				  int *num_alarms)
{
	const sensors_subfeature *sf;

	for (; sfl->subfeature >= 0; sfl++) {
		sf = sensors_get_subfeature(name, feature, sfl->subfeature);
		if (sf) {
			if (sfl->alarm) {
				/*
				 * Only queue alarm subfeatures if the alarm
				 * is active, and don't store the alarm value
				 * (it is implied to be active if queued).
				 */
				if (get_value(name, sf)) {
					alarms[*num_alarms].name = sfl->name;
					(*num_alarms)++;
				}
			} else {
				/*
				 * Always queue limit subfeatures with their value.
				 */
				limits[*num_limits].value = get_value(name, sf);
				limits[*num_limits].name = sfl->name;
				(*num_limits)++;
			}
			if (sfl->exists) {
				get_sensor_limit_data(name, feature, sfl->exists,
						      limits, num_limits,
						      alarms, num_alarms);
			}
		}
	}
}

static const struct sensor_subfeature_list temp_min_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_MIN_HYST, NULL, 0, hyst_str },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list temp_lcrit_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_LCRIT_HYST, NULL, 0, hyst_str },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list temp_max_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_MAX_HYST, NULL, 0, hyst_str },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list temp_crit_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_CRIT_HYST, NULL, 0, hyst_str },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list temp_emergency_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_EMERGENCY_HYST, NULL, 0,
	    hyst_str },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list temp_sensors[] = {
	{ SENSORS_SUBFEATURE_TEMP_ALARM, NULL, 1, NULL },
	{ SENSORS_SUBFEATURE_TEMP_LCRIT_ALARM, NULL, 1, "LCRIT" },
	{ SENSORS_SUBFEATURE_TEMP_MIN_ALARM, NULL, 1, "LOW" },
	{ SENSORS_SUBFEATURE_TEMP_MAX_ALARM, NULL, 1, "HIGH" },
	{ SENSORS_SUBFEATURE_TEMP_CRIT_ALARM, NULL, 1, "CRIT" },
	{ SENSORS_SUBFEATURE_TEMP_EMERGENCY_ALARM, NULL, 1, "EMERGENCY" },
	{ SENSORS_SUBFEATURE_TEMP_MIN, temp_min_sensors, 0, "low" },
	{ SENSORS_SUBFEATURE_TEMP_MAX, temp_max_sensors, 0, "high" },
	{ SENSORS_SUBFEATURE_TEMP_LCRIT, temp_lcrit_sensors, 0, "crit low" },
	{ SENSORS_SUBFEATURE_TEMP_CRIT, temp_crit_sensors, 0, "crit" },
	{ SENSORS_SUBFEATURE_TEMP_EMERGENCY, temp_emergency_sensors, 0,
	    "emerg" },
	{ SENSORS_SUBFEATURE_TEMP_LOWEST, NULL, 0, "lowest" },
	{ SENSORS_SUBFEATURE_TEMP_HIGHEST, NULL, 0, "highest" },
	{ -1, NULL, 0, NULL }
};

#define NUM_TEMP_ALARMS		6
#define NUM_TEMP_SENSORS	(ARRAY_SIZE(temp_sensors) \
				 + ARRAY_SIZE(temp_max_sensors) \
				 + ARRAY_SIZE(temp_crit_sensors) \
				 + ARRAY_SIZE(temp_emergency_sensors) \
				 - NUM_TEMP_ALARMS - 4)

static void print_chip_temp(const sensors_chip_name *name,
			    const sensors_feature *feature,
			    int label_size)
{
	struct sensor_subfeature_data sensors[NUM_TEMP_SENSORS];
	struct sensor_subfeature_data alarms[NUM_TEMP_ALARMS];
	int sensor_count, alarm_count;
	const sensors_subfeature *sf;
	double val;
	char *label;
	int i;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_TEMP_FAULT);
	if (sf && get_value(name, sf)) {
		printf("   FAULT  ");
	} else {
		sf = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_TEMP_INPUT);
		if (sf && get_input_value(name, sf, &val) == 0) {
			get_input_value(name, sf, &val);
			if (fahrenheit)
				val = deg_ctof(val);
			printf("%+6.1f%s  ", val, degstr);
		} else
			printf("     N/A  ");
	}

	sensor_count = alarm_count = 0;
	get_sensor_limit_data(name, feature, temp_sensors,
			      sensors, &sensor_count, alarms, &alarm_count);

	for (i = 0; i < sensor_count; i++) {
		if (fahrenheit)
			sensors[i].value = deg_ctof(sensors[i].value);
		sensors[i].unit = degstr;
	}

	print_limits(sensors, sensor_count, alarms, alarm_count, label_size,
		     "%-4s = %+5.1f%s");

	/* print out temperature sensor info */
	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_TEMP_TYPE);
	if (sf) {
		int sens = (int)get_value(name, sf);

		/* older kernels / drivers sometimes report a beta value for
		   thermistors */
		if (sens > 1000)
			sens = 4;

		printf("  sensor = %s", sens == 0 ? "disabled" :
		       sens == 1 ? "CPU diode" :
		       sens == 2 ? "transistor" :
		       sens == 3 ? "thermal diode" :
		       sens == 4 ? "thermistor" :
		       sens == 5 ? "AMD AMDSI" :
		       sens == 6 ? "Intel PECI" : "unknown");
	}
	printf("\n");
}

static const struct sensor_subfeature_list voltage_sensors[] = {
	{ SENSORS_SUBFEATURE_IN_ALARM, NULL, 1, NULL },
	{ SENSORS_SUBFEATURE_IN_LCRIT_ALARM, NULL, 1, "LCRIT" },
	{ SENSORS_SUBFEATURE_IN_MIN_ALARM, NULL, 1, "MIN" },
	{ SENSORS_SUBFEATURE_IN_MAX_ALARM, NULL, 1, "MAX" },
	{ SENSORS_SUBFEATURE_IN_CRIT_ALARM, NULL, 1, "CRIT" },
	{ SENSORS_SUBFEATURE_IN_LCRIT, NULL, 0, "crit min" },
	{ SENSORS_SUBFEATURE_IN_MIN, NULL, 0, "min" },
	{ SENSORS_SUBFEATURE_IN_MAX, NULL, 0, "max" },
	{ SENSORS_SUBFEATURE_IN_CRIT, NULL, 0, "crit max" },
	{ SENSORS_SUBFEATURE_IN_AVERAGE, NULL, 0, "avg" },
	{ SENSORS_SUBFEATURE_IN_LOWEST, NULL, 0, "lowest" },
	{ SENSORS_SUBFEATURE_IN_HIGHEST, NULL, 0, "highest" },
	{ -1, NULL, 0, NULL }
};

#define NUM_IN_ALARMS	5
#define NUM_IN_SENSORS	(ARRAY_SIZE(voltage_sensors) - NUM_IN_ALARMS - 1)

static void print_chip_in(const sensors_chip_name *name,
			  const sensors_feature *feature,
			  int label_size)
{
	const sensors_subfeature *sf;
	char *label;
	struct sensor_subfeature_data sensors[NUM_IN_SENSORS];
	struct sensor_subfeature_data alarms[NUM_IN_ALARMS];
	int sensor_count, alarm_count;
	double val;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_IN_INPUT);
	if (sf && get_input_value(name, sf, &val) == 0)
		printf("%+6.2f V  ", val);
	else
		printf("     N/A  ");

	sensor_count = alarm_count = 0;
	get_sensor_limit_data(name, feature, voltage_sensors,
			      sensors, &sensor_count, alarms, &alarm_count);

	print_limits(sensors, sensor_count, alarms, alarm_count, label_size,
		     "%s = %+6.2f V");

	printf("\n");
}

static void print_chip_fan(const sensors_chip_name *name,
			   const sensors_feature *feature,
			   int label_size)
{
	const sensors_subfeature *sf, *sfmin, *sfmax, *sfdiv;
	double val;
	char *label;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_FAN_FAULT);
	if (sf && get_value(name, sf))
		printf("   FAULT");
	else {
		sf = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_FAN_INPUT);
		if (sf && get_input_value(name, sf, &val) == 0)
			printf("%4.0f RPM", val);
		else
			printf("     N/A");
	}

	sfmin = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_MIN);
	sfmax = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_MAX);
	sfdiv = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_DIV);
	if (sfmin || sfmax || sfdiv) {
		printf("  (");
		if (sfmin)
			printf("min = %4.0f RPM",
			       get_value(name, sfmin));
		if (sfmax)
			printf("%smax = %4.0f RPM",
			       sfmin ? ", " : "",
			       get_value(name, sfmax));
		if (sfdiv)
			printf("%sdiv = %1.0f",
			       (sfmin || sfmax) ? ", " : "",
			       get_value(name, sfdiv));
		printf(")");
	}

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_FAN_ALARM);
	sfmin = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_MIN_ALARM);
	sfmax = sensors_get_subfeature(name, feature,
				       SENSORS_SUBFEATURE_FAN_MAX_ALARM);
	if ((sf && get_value(name, sf)) ||
	    (sfmin && get_value(name, sfmin)) ||
	    (sfmax && get_value(name, sfmax)))
		printf("  ALARM");

	printf("\n");
}

struct scale_table {
	double upper_bound;
	const char *unit;
};

static void scale_value(double *value, const char **prefixstr)
{
	double abs_value = fabs(*value);
	double divisor = 1e-9;
	static struct scale_table prefix_scales[] = {
		{1e-6, "n"},
		{1e-3, "u"},
		{1,    "m"},
		{1e3,   ""},
		{1e6,  "k"},
		{1e9,  "M"},
		{0,    "G"}, /* no upper bound */
	};
	struct scale_table *scale = prefix_scales;

	if (abs_value == 0) {
		*prefixstr = "";
		return;
	}

	while (scale->upper_bound && abs_value > scale->upper_bound) {
		divisor = scale->upper_bound;
		scale++;
	}

	*value /= divisor;
	*prefixstr = scale->unit;
}

static const struct sensor_subfeature_list power_common_sensors[] = {
	{ SENSORS_SUBFEATURE_POWER_ALARM, NULL, 1, NULL },
	{ SENSORS_SUBFEATURE_POWER_MAX_ALARM, NULL, 1, "MAX" },
	{ SENSORS_SUBFEATURE_POWER_CRIT_ALARM, NULL, 1, "CRIT" },
	{ SENSORS_SUBFEATURE_POWER_CAP_ALARM, NULL, 1, "CAP" },
	{ SENSORS_SUBFEATURE_POWER_MAX, NULL, 0, "max" },
	{ SENSORS_SUBFEATURE_POWER_CRIT, NULL, 0, "crit" },
	{ SENSORS_SUBFEATURE_POWER_CAP, NULL, 0, "cap" },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list power_inst_sensors[] = {
	{ SENSORS_SUBFEATURE_POWER_INPUT_LOWEST, NULL, 0, "lowest" },
	{ SENSORS_SUBFEATURE_POWER_INPUT_HIGHEST, NULL, 0, "highest" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE, NULL, 0, "avg" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_LOWEST, NULL, 0, "avg lowest" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_HIGHEST, NULL, 0, "avg highest" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL, NULL, 0,
		"interval" },
	{ -1, NULL, 0, NULL }
};

static const struct sensor_subfeature_list power_avg_sensors[] = {
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_LOWEST, NULL, 0, "lowest" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_HIGHEST, NULL, 0, "highest" },
	{ SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL, NULL, 0,
		"interval" },
	{ -1, NULL, 0, NULL }
};

#define NUM_POWER_ALARMS	4
#define NUM_POWER_SENSORS	(ARRAY_SIZE(power_common_sensors) \
				 + ARRAY_SIZE(power_inst_sensors) \
				 - NUM_POWER_ALARMS - 2)

static void print_chip_power(const sensors_chip_name *name,
			     const sensors_feature *feature,
			     int label_size)
{
	double val;
	const sensors_subfeature *sf;
	struct sensor_subfeature_data sensors[NUM_POWER_SENSORS];
	struct sensor_subfeature_data alarms[NUM_POWER_ALARMS];
	int sensor_count, alarm_count;
	char *label;
	const char *unit;
	int i;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sensor_count = alarm_count = 0;

	/*
	 * Power sensors come in 2 flavors: instantaneous and averaged.
	 * Most devices only support one flavor, so we try to display the
	 * average power if the instantaneous power attribute does not exist.
	 * If both instantaneous power and average power are supported,
	 * average power is displayed as limit.
	 */
	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_POWER_INPUT);
	get_sensor_limit_data(name, feature,
			      sf ? power_inst_sensors : power_avg_sensors,
			      sensors, &sensor_count, alarms, &alarm_count);
	/* Add sensors common to both flavors. */
	get_sensor_limit_data(name, feature, power_common_sensors,
			      sensors, &sensor_count, alarms, &alarm_count);
	if (!sf)
		sf = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_POWER_AVERAGE);

	if (sf && get_input_value(name, sf, &val) == 0) {
		scale_value(&val, &unit);
		printf("%6.2f %sW%*s", val, unit, 2 - (int)strlen(unit), "");
	} else
		printf("     N/A  ");

	for (i = 0; i < sensor_count; i++) {
		/*
		 * Unit is W and needs to be scaled for all attributes except
		 * interval, which does not need to be scaled and is reported in
		 * seconds.
		 */
		if (strcmp(sensors[i].name, "interval")) {
			char *tmpstr;

			tmpstr = alloca(4);
			scale_value(&sensors[i].value, &unit);
			snprintf(tmpstr, 4, "%sW", unit);
			sensors[i].unit = tmpstr;
		} else {
			sensors[i].unit = "s";
		}
	}
	print_limits(sensors, sensor_count, alarms, alarm_count,
		     label_size, "%s = %6.2f %s");

	printf("\n");
}

static void print_chip_energy(const sensors_chip_name *name,
			      const sensors_feature *feature,
			      int label_size)
{
	double val;
	const sensors_subfeature *sf;
	char *label;
	const char *unit;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_ENERGY_INPUT);
	if (sf && get_input_value(name, sf, &val) == 0) {
		scale_value(&val, &unit);
		printf("%6.2f %sJ", val, unit);
	} else
		printf("     N/A");

	printf("\n");
}

static void print_chip_vid(const sensors_chip_name *name,
			   const sensors_feature *feature,
			   int label_size)
{
	char *label;
	const sensors_subfeature *subfeature;
	double vid;

	subfeature = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_VID);
	if (!subfeature)
		return;

	if ((label = sensors_get_label(name, feature))
	 && !sensors_get_value(name, subfeature->number, &vid)) {
		print_label(label, label_size);
		printf("%+6.3f V\n", vid);
	}
	free(label);
}

static void print_chip_humidity(const sensors_chip_name *name,
				const sensors_feature *feature,
				int label_size)
{
	char *label;
	const sensors_subfeature *subfeature;
	double humidity;

	subfeature = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_HUMIDITY_INPUT);
	if (!subfeature)
		return;

	if ((label = sensors_get_label(name, feature))
	 && !sensors_get_value(name, subfeature->number, &humidity)) {
		print_label(label, label_size);
		printf("%6.1f %%RH\n", humidity);
	}
	free(label);
}

static void print_chip_beep_enable(const sensors_chip_name *name,
				   const sensors_feature *feature,
				   int label_size)
{
	char *label;
	const sensors_subfeature *subfeature;
	double beep_enable;

	subfeature = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_BEEP_ENABLE);
	if (!subfeature)
		return;

	if ((label = sensors_get_label(name, feature))
	 && !sensors_get_value(name, subfeature->number, &beep_enable)) {
		print_label(label, label_size);
		printf("%s\n", beep_enable ? "enabled" : "disabled");
	}
	free(label);
}

static const struct sensor_subfeature_list current_sensors[] = {
	{ SENSORS_SUBFEATURE_CURR_ALARM, NULL, 1, NULL },
	{ SENSORS_SUBFEATURE_CURR_LCRIT_ALARM, NULL, 1, "LCRIT" },
	{ SENSORS_SUBFEATURE_CURR_MIN_ALARM, NULL, 1, "MIN" },
	{ SENSORS_SUBFEATURE_CURR_MAX_ALARM, NULL, 1, "MAX" },
	{ SENSORS_SUBFEATURE_CURR_CRIT_ALARM, NULL, 1, "CRIT" },
	{ SENSORS_SUBFEATURE_CURR_LCRIT, NULL, 0, "crit min" },
	{ SENSORS_SUBFEATURE_CURR_MIN, NULL, 0, "min" },
	{ SENSORS_SUBFEATURE_CURR_MAX, NULL, 0, "max" },
	{ SENSORS_SUBFEATURE_CURR_CRIT, NULL, 0, "crit max" },
	{ SENSORS_SUBFEATURE_CURR_AVERAGE, NULL, 0, "avg" },
	{ SENSORS_SUBFEATURE_CURR_LOWEST, NULL, 0, "lowest" },
	{ SENSORS_SUBFEATURE_CURR_HIGHEST, NULL, 0, "highest" },
	{ -1, NULL, 0, NULL }
};

#define NUM_CURR_ALARMS		5
#define NUM_CURR_SENSORS	(ARRAY_SIZE(current_sensors) - NUM_CURR_ALARMS - 1)

static void print_chip_curr(const sensors_chip_name *name,
			    const sensors_feature *feature,
			    int label_size)
{
	const sensors_subfeature *sf;
	double val;
	char *label;
	struct sensor_subfeature_data sensors[NUM_CURR_SENSORS];
	struct sensor_subfeature_data alarms[NUM_CURR_ALARMS];
	int sensor_count, alarm_count;

	if (!(label = sensors_get_label(name, feature))) {
		fprintf(stderr, "ERROR: Can't get label of feature %s!\n",
			feature->name);
		return;
	}
	print_label(label, label_size);
	free(label);

	sf = sensors_get_subfeature(name, feature,
				    SENSORS_SUBFEATURE_CURR_INPUT);
	if (sf && get_input_value(name, sf, &val) == 0)
		printf("%+6.2f A  ", val);
	else
		printf("     N/A  ");

	sensor_count = alarm_count = 0;
	get_sensor_limit_data(name, feature, current_sensors,
			      sensors, &sensor_count, alarms, &alarm_count);

	print_limits(sensors, sensor_count, alarms, alarm_count, label_size,
		     "%s = %+6.2f A");

	printf("\n");
}

static void print_chip_intrusion(const sensors_chip_name *name,
				 const sensors_feature *feature,
				 int label_size)
{
	char *label;
	const sensors_subfeature *subfeature;
	double alarm;

	subfeature = sensors_get_subfeature(name, feature,
					    SENSORS_SUBFEATURE_INTRUSION_ALARM);
	if (!subfeature)
		return;

	if ((label = sensors_get_label(name, feature))
	 && !sensors_get_value(name, subfeature->number, &alarm)) {
		print_label(label, label_size);
		printf("%s\n", alarm ? "ALARM" : "OK");
	}
	free(label);
}

void print_chip(const sensors_chip_name *name)
{
	const sensors_feature *feature;
	int i, label_size;

	label_size = get_label_size(name);

	i = 0;
	while ((feature = sensors_get_features(name, &i))) {
		switch (feature->type) {
		case SENSORS_FEATURE_TEMP:
			print_chip_temp(name, feature, label_size);
			break;
		case SENSORS_FEATURE_IN:
			print_chip_in(name, feature, label_size);
			break;
		case SENSORS_FEATURE_FAN:
			print_chip_fan(name, feature, label_size);
			break;
		case SENSORS_FEATURE_VID:
			print_chip_vid(name, feature, label_size);
			break;
		case SENSORS_FEATURE_BEEP_ENABLE:
			print_chip_beep_enable(name, feature, label_size);
			break;
		case SENSORS_FEATURE_POWER:
			print_chip_power(name, feature, label_size);
			break;
		case SENSORS_FEATURE_ENERGY:
			print_chip_energy(name, feature, label_size);
			break;
		case SENSORS_FEATURE_CURR:
			print_chip_curr(name, feature, label_size);
			break;
		case SENSORS_FEATURE_INTRUSION:
			print_chip_intrusion(name, feature, label_size);
			break;
		case SENSORS_FEATURE_HUMIDITY:
			print_chip_humidity(name, feature, label_size);
			break;
		default:
			continue;
		}
	}
}
