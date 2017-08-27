/*
 * rd_sensors.c: Read sensors statistics
 * (C) 1999-2011 by Sebastien GODARD (sysstat <at> orange.fr)
 *
 ***************************************************************************
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published  by  the *
 * Free Software Foundation; either version 2 of the License, or (at  your *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it  will  be  useful,  but *
 * WITHOUT ANY WARRANTY; without the implied warranty  of  MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License *
 * for more details.                                                       *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   *
 ***************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "rd_sensors.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

#ifdef HAVE_SENSORS
#include "sensors/sensors.h"
#endif

/*
 ***************************************************************************
 * Read fan statistics.
 *
 * IN:
 * @st_pwr_fan	Structure where stats will be saved.
 * @nbr		Total number of fans.
 *
 * OUT:
 * @st_pwr_fan Structure with statistics.
 ***************************************************************************
 */
void read_fan(struct stats_pwr_fan *st_pwr_fan, int nbr)
{
#ifdef HAVE_SENSORS
	int count = 0;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	const sensors_subfeature *sub;
	struct stats_pwr_fan *st_pwr_fan_i;
	int chip_nr = 0;
	int i, j;

	memset(st_pwr_fan, 0, STATS_PWR_FAN_SIZE);
	int err = 0;

	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {
		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {
			if ((feature->type == SENSORS_FEATURE_FAN) && (count < nbr)) {
				j = 0;
				st_pwr_fan_i = st_pwr_fan + count;
				sensors_snprintf_chip_name(st_pwr_fan_i->device, MAX_SENSORS_DEV_LEN, chip);
				
				while ((sub = sensors_get_all_subfeatures(chip, feature, &j))) {
					if ((sub->type == SENSORS_SUBFEATURE_FAN_INPUT) &&
					    (sub->flags & SENSORS_MODE_R)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_fan_i->rpm))) {
							st_pwr_fan_i->rpm = 0;
						}
					}
					else if ((sub->type == SENSORS_SUBFEATURE_FAN_MIN)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_fan_i->rpm_min))) {
							st_pwr_fan_i->rpm_min = 0;
						}
					}
				}
				count++;
			}
		}
	}
#endif /* HAVE_SENSORS */
}

/*
 ***************************************************************************
 * Read device temperature statistics.
 *
 * IN:
 * @st_pwr_temp	Structure where stats will be saved.
 * @nbr		Total number of fans.
 *
 * OUT:
 * @st_pwr_temp	Structure with statistics.
 ***************************************************************************
 */
void read_temp(struct stats_pwr_temp *st_pwr_temp, int nbr)
{
#ifdef HAVE_SENSORS
	int count = 0;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	const sensors_subfeature *sub;
	struct stats_pwr_temp *st_pwr_temp_i;
	int chip_nr = 0;
	int i, j;

	memset(st_pwr_temp, 0, STATS_PWR_TEMP_SIZE);
	int err = 0;

	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {
		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {
			if ((feature->type == SENSORS_FEATURE_TEMP) && (count < nbr)) {
				j = 0;
				st_pwr_temp_i = st_pwr_temp + count;
				sensors_snprintf_chip_name(st_pwr_temp_i->device, MAX_SENSORS_DEV_LEN, chip);
				
				while ((sub = sensors_get_all_subfeatures(chip, feature, &j))) {
					if ((sub->type == SENSORS_SUBFEATURE_TEMP_INPUT) &&
						(sub->flags & SENSORS_MODE_R)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_temp_i->temp))) {
							st_pwr_temp_i->temp = 0;
						}
					}
					else if ((sub->type == SENSORS_SUBFEATURE_TEMP_MIN)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_temp_i->temp_min))) {
							st_pwr_temp_i->temp_min = 0;
						}
					}
					else if ((sub->type == SENSORS_SUBFEATURE_TEMP_MAX)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_temp_i->temp_max))) {
							st_pwr_temp_i->temp_max = 0;
						}
					}
				}
				count++;
			}
		}
	}
#endif /* HAVE_SENSORS */
}

/*
 ***************************************************************************
 * Read voltage inputs statistics.
 *
 * IN:
 * @st_pwr_in	Structure where stats will be saved.
 * @nbr		Total number of voltage inputs.
 *
 * OUT:
 * @st_pwr_in	Structure with statistics.
 ***************************************************************************
 */
void read_in(struct stats_pwr_in *st_pwr_in, int nbr)
{
#ifdef HAVE_SENSORS
	int count = 0;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	const sensors_subfeature *sub;
	struct stats_pwr_in *st_pwr_in_i;
	int chip_nr = 0;
	int i, j;

	memset(st_pwr_in, 0, STATS_PWR_IN_SIZE);
	int err = 0;

	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {
		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {
			if ((feature->type == SENSORS_FEATURE_IN) && (count < nbr)) {
				j = 0;
				st_pwr_in_i = st_pwr_in + count;
				sensors_snprintf_chip_name(st_pwr_in_i->device, MAX_SENSORS_DEV_LEN, chip);

				while ((sub = sensors_get_all_subfeatures(chip, feature, &j))) {
					if ((sub->type == SENSORS_SUBFEATURE_IN_INPUT) &&
						(sub->flags & SENSORS_MODE_R)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_in_i->in))) {
							st_pwr_in_i->in = 0;
						}
					}
					else if ((sub->type == SENSORS_SUBFEATURE_IN_MIN)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_in_i->in_min))) {
							st_pwr_in_i->in_min = 0;
						}
					}
					else if ((sub->type == SENSORS_SUBFEATURE_IN_MAX)) {
						if ((err = sensors_get_value(chip, sub->number, &st_pwr_in_i->in_max))) {
							st_pwr_in_i->in_max = 0;
						}
					}
				}
				count++;
			}
		}
	}
#endif /* HAVE_SENSORS */
}

#ifdef HAVE_SENSORS
/*
 ***************************************************************************
 * Count the number of sensors of given type on the machine.
 *
 * IN:
 * @type	Type of sensors.
 *
 * RETURNS:
 * Number of sensors.
 ***************************************************************************
 */
int get_sensors_nr(sensors_feature_type type) {
	int count = 0;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	int chip_nr = 0;
	int i;

	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {
		i = 0;
		while ((feature = sensors_get_features(chip, &i))) {
			if (feature->type == type) {
				count++;
			}
		}
	}

	return count;
}
#endif /* HAVE_SENSORS */

/*
 ***************************************************************************
 * Count the number of fans on the machine.
 *
 * RETURNS:
 * Number of fans.
 ***************************************************************************
 */
int get_fan_nr(void)
{
#ifdef HAVE_SENSORS
	return get_sensors_nr(SENSORS_FEATURE_FAN);
#else
	return 0;
#endif /* HAVE_SENSORS */
}

/*
 ***************************************************************************
 * Count the number of temperature sensors on the machine.
 *
 * RETURNS:
 * Number of temperature sensors.
 ***************************************************************************
 */
int get_temp_nr(void)
{
#ifdef HAVE_SENSORS
	return get_sensors_nr(SENSORS_FEATURE_TEMP);
#else
	return 0;
#endif /* HAVE_SENSORS */

}

/*
 ***************************************************************************
 * Count the number of voltage inputs on the machine.
 *
 * RETURNS:
 * Number of voltage inputs.
 ***************************************************************************
 */
int get_in_nr(void)
{
#ifdef HAVE_SENSORS
	return get_sensors_nr(SENSORS_FEATURE_IN);
#else
	return 0;
#endif /* HAVE_SENSORS */

}
