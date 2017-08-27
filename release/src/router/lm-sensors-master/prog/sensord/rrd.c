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

/*
 * RRD is the Round Robin Database
 *
 * Get this package from:
 *   http://people.ee.ethz.ch/~oetiker/webtools/rrdtool/
 *
 * For compilation you need the development libraries;
 * for execution you need the runtime libraries; for
 * Web-based graph access you need the binary rrdtool.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <rrd.h>

#include "args.h"
#include "sensord.h"

#define DO_READ 0
#define DO_SCAN 1
#define DO_SET 2
#define DO_RRD 3

/* one integer */
#define STEP_BUFF 64
/* RRA:AVERAGE:0.5:1:12345 */
#define RRA_BUFF 256
/* weak: max sensors for RRD .. TODO: fix */
#define MAX_RRD_SENSORS 256
/* weak: max raw label length .. TODO: fix */
#define RAW_LABEL_LENGTH 32
/* DS:label:GAUGE:900:U:U | :3000 .. TODO: fix */
#define RRD_BUFF 64

char rrdBuff[MAX_RRD_SENSORS * RRD_BUFF + 1];
static char rrdLabels[MAX_RRD_SENSORS][RAW_LABEL_LENGTH + 1];

#define LOADAVG "loadavg"
#define LOAD_AVERAGE "Load Average"

typedef void (*FeatureFN) (void *data, const char *rawLabel, const char *label,
			   const FeatureDescriptor *feature);

static char rrdNextChar(char c)
{
	if (c == '9') {
		return 'A';
	} else if (c == 'Z') {
		return 'a';
	} else if (c == 'z') {
		return 0;
	} else {
		return c + 1;
	}
}

static void rrdCheckLabel(const char *rawLabel, int index0)
{
	char *buffer = rrdLabels[index0];
	int i, j, okay;

	i = 0;
	/* contrain raw label to [A-Za-z0-9_] */
	while ((i < RAW_LABEL_LENGTH) && rawLabel[i]) {
		char c = rawLabel[i];
		if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))
		    || ((c >= '0') && (c <= '9')) || (c == '_')) {
			buffer[i] = c;
		} else {
			buffer[i] = '_';
		}
		++i;
	}
	buffer[i] = '\0';

	j = 0;
	okay = (i > 0);

	/* locate duplicates */
	while (okay && (j < index0))
		okay = strcmp(rrdLabels[j++], buffer);

	/* uniquify duplicate labels with _? or _?? */
	while (!okay) {
		if (!buffer[i]) {
			if (i > RAW_LABEL_LENGTH - 3)
				i = RAW_LABEL_LENGTH - 3;
			buffer[i] = '_';
			buffer[i + 1] = '0';
			buffer[i + 2] = '\0';
		} else if (!buffer[i + 2]) {
			if (!(buffer[i + 1] = rrdNextChar(buffer[i + 1]))) {
				buffer[i + 1] = '0';
				buffer[i + 2] = '0';
				buffer[i + 3] = '\0';
			}
		} else {
			if (!(buffer[i + 2] = rrdNextChar(buffer[i + 2]))) {
				buffer[i + 1] = rrdNextChar(buffer[i + 1]);
				buffer[i + 2] = '0';
			}
		}
		j = 0;
		okay = 1;
		while (okay && (j < index0))
			okay = strcmp(rrdLabels[j ++], buffer);
	}
}

/* Returns the number of features processed, or -1 on error */
static int _applyToFeatures(FeatureFN fn, void *data,
			    const sensors_chip_name *chip,
			    const ChipDescriptor *desc,
			    int labelOffset)
{
	int i;
	const FeatureDescriptor *features = desc->features;
	const FeatureDescriptor *feature;
	const char *rawLabel;
	char *label;

	for (i = 0; labelOffset + i < MAX_RRD_SENSORS && features[i].format; ++i) {
		feature = features + i;
		rawLabel = feature->feature->name;

		label = sensors_get_label(chip, feature->feature);
		if (!label) {
			sensorLog(LOG_ERR, "Error getting sensor label: %s/%s",
				  chip->prefix, rawLabel);
			return -1;
		}

		rrdCheckLabel(rawLabel, labelOffset + i);
		fn(data, rrdLabels[labelOffset + i], label, feature);
		free(label);
	}
	return i;
}

static ChipDescriptor *lookup_known_chips(const sensors_chip_name *chip)
{
	int i;

	/* Trick: we compare addresses here. We know it works
	 * because both pointers were returned by
	 * sensors_get_detected_chips(), so they refer to
	 * libsensors internal structures, which do not move.
	 */
	for (i = 0; knownChips[i].features; i++) {
		if (knownChips[i].name == chip) {
			return &knownChips[i];
		}
	}
	return NULL;
}

static int applyToFeatures(FeatureFN fn, void *data)
{
	int i, i_detected, ret, labelOffset = 0;
	const sensors_chip_name *chip, *chip_arg;
	ChipDescriptor *desc;

	for (i = 0; i < sensord_args.numChipNames; i++) {
		chip_arg = &sensord_args.chipNames[i];
		i_detected = 0;
		while ((chip = sensors_get_detected_chips(chip_arg,
							  &i_detected))) {
			desc = lookup_known_chips(chip);
			if (!desc)
				continue;

			ret = _applyToFeatures(fn, data, chip, desc, labelOffset);
			if (ret < 0)
				return ret;
			labelOffset += ret;
		}
	}
	return 0;
}

struct ds {
	int num;
	const char **argv;
};

static void rrdGetSensors_DS(void *_data, const char *rawLabel,
			     const char *label,
			     const FeatureDescriptor *feature)
{
	(void) label; /* no warning */
	if (!feature || feature->rrd) {
		struct ds *data = _data;
		char *ptr = rrdBuff + data->num * RRD_BUFF;
		const char *min, *max;
		data->argv[data->num ++] = ptr;

		/* arbitrary sanity limits */
		switch (feature ? feature->type : DataType_other) {
		case DataType_voltage:
			min="-25";
			max="25";
			break;
		case DataType_rpm:
			min = "0";
			max = "12000";
			break;
		case DataType_temperature:
			min = "-100";
			max = "250";
			break;
		default:
			min = max = "U";
			break;
		}

		/*
		 * number of seconds downtime during which average be used
		 * instead of unknown
		 */
		sprintf(ptr, "DS:%s:GAUGE:%d:%s:%s", rawLabel, 5 *
			sensord_args.rrdTime, min, max);
	}
}

static int rrdGetSensors(const char **argv)
{
	int ret = 0;
	struct ds data = { 0, argv};
	ret = applyToFeatures(rrdGetSensors_DS, &data);
	if (!ret && sensord_args.doLoad)
		rrdGetSensors_DS(&data, LOADAVG, LOAD_AVERAGE, NULL);
	return ret ? -1 : data.num;
}

int rrdInit(void)
{
	int ret;
	struct stat sb;
	char stepBuff[STEP_BUFF], rraBuff[RRA_BUFF];
	int argc = 4, num;
	const char *argv[6 + MAX_RRD_SENSORS] = {
		"sensord", sensord_args.rrdFile, "-s", stepBuff
	};

	sensorLog(LOG_DEBUG, "sensor RRD init");

	/* Create RRD if it does not exist. */
	if (stat(sensord_args.rrdFile, &sb)) {
		if (errno != ENOENT) {
			sensorLog(LOG_ERR, "Could not stat rrd file: %s\n",
				  sensord_args.rrdFile);
			return -1;
		}
		sensorLog(LOG_INFO, "Creating round robin database");

		num = rrdGetSensors(argv + argc);
		if (num < 1) {
			sensorLog(LOG_ERR, "Error creating RRD: %s: %s",
				  sensord_args.rrdFile, "No sensors detected");
			return -1;
		}

		sprintf(stepBuff, "%d", sensord_args.rrdTime);
		sprintf(rraBuff, "RRA:%s:%f:%d:%d",
			sensord_args.rrdNoAverage ? "LAST" :"AVERAGE",
			0.5, 1, 7 * 24 * 60 * 60 / sensord_args.rrdTime);

		argc += num;
		argv[argc++] = rraBuff;
		argv[argc] = NULL;

		ret = rrd_create(argc, (char**) argv);
		if (ret == -1) {
			sensorLog(LOG_ERR, "Error creating RRD file: %s: %s",
				  sensord_args.rrdFile, rrd_get_error());
			return -1;
		}
	}

	sensorLog(LOG_DEBUG, "sensor RRD initialized");
	return 0;
}

#define RRDCGI "/usr/bin/rrdcgi"
#define WWWDIR "/sensord"

struct gr {
	DataType type;
	const char *h2;
	const char *image;
	const char *title;
	const char *axisTitle;
	const char *axisDefn;
	const char *options;
	int loadAvg;
};

static void rrdCGI_DEF(void *_data, const char *rawLabel, const char *label,
		       const FeatureDescriptor *feature)
{
	struct gr *data = _data;
	(void) label; /* no warning */
	if (!feature || (feature->rrd && (feature->type == data->type)))
		printf("\n\tDEF:%s=%s:%s:AVERAGE", rawLabel,
		       sensord_args.rrdFile, rawLabel);
}

/*
 * Compute an arbitrary color based on the sensor label. This is preferred
 * over a random value because this guarantees that daily and weekly charts
 * will use the same colors.
 */
static int rrdCGI_color(const char *label)
{
	unsigned long color = 0, brightness;
	const char *c;

	for (c = label; *c; c++) {
		color = (color << 6) + (color >> (*c & 7));
		color ^= (*c) * 0x401;
	}
	color &= 0xffffff;
	/* Adjust very light colors */
	brightness = (color & 0xff) + ((color >> 8) & 0xff) + (color >> 16);
	if (brightness > 672)
		color &= 0x7f7f7f;
	/* Adjust very dark colors */
	else if (brightness < 96)
		color |= 0x808080;
	return color;
}

static void rrdCGI_LINE(void *_data, const char *rawLabel, const char *label,
			const FeatureDescriptor *feature)
{
	struct gr *data = _data;
	if (!feature || (feature->rrd && (feature->type == data->type)))
		printf("\n\tLINE2:%s#%.6x:\"%s\"", rawLabel,
		       rrdCGI_color(label), label);
}

static struct gr graphs[] = {
	{
		DataType_temperature,
		"Daily Temperature Summary",
		"daily-temperature",
		"Temperature",
		"Temperature (C)",
		"HOUR:1:HOUR:3:HOUR:3:0:%b %d %H:00",
		"-s -1d -l 0",
		1
	}, {
		DataType_rpm,
		"Daily Fan Speed Summary",
		"daily-rpm",
		"Fan Speed",
		"Speed (RPM)",
		"HOUR:1:HOUR:3:HOUR:3:0:%b %d %H:00",
		"-s -1d -l 0 -X 0",
		0
	}, {
		DataType_voltage,
		"Daily Voltage Summary",
		"daily-voltage",
		"Power Supply",
		"Voltage (V)",
		"HOUR:1:HOUR:3:HOUR:3:0:%b %d %H:00",
		"-s -1d --alt-autoscale",
		0
	}, {
		DataType_temperature,
		"Weekly Temperature Summary",
		"weekly-temperature",
		"Temperature",
		"Temperature (C)",
		"HOUR:6:DAY:1:DAY:1:86400:%a %b %d",
		"-s -1w -l 0",
		1
	}, {
		DataType_rpm,
		"Weekly Fan Speed Summary",
		"weekly-rpm",
		"Fan Speed",
		"Speed (RPM)",
		"HOUR:6:DAY:1:DAY:1:86400:%a %b %d",
		"-s -1w -l 0 -X 0",
		0
	}, {
		DataType_voltage,
		"Weekly Voltage Summary",
		"weekly-voltage",
		"Power Supply",
		"Voltage (V)",
		"HOUR:6:DAY:1:DAY:1:86400:%a %b %d",
		"-s -1w --alt-autoscale",
		0
	}
};

int rrdUpdate(void)
{
	int ret = rrdChips ();

	if (!ret && sensord_args.doLoad) {
		FILE *loadavg;
		if (!(loadavg = fopen("/proc/loadavg", "r"))) {
			sensorLog(LOG_ERR,
				  "Error opening `/proc/loadavg': %s",
				  strerror(errno));
			ret = 1;
		} else {
			float value;
			if (fscanf(loadavg, "%f", &value) != 1) {
				sensorLog(LOG_ERR,
					  "Error reading load average");
				ret = 2;
			} else {
				sprintf(rrdBuff + strlen(rrdBuff), ":%f",
					value);
			}
			fclose(loadavg);
		}
	}
	if (!ret) {
		const char *argv[] = {
			"sensord", sensord_args.rrdFile, rrdBuff, NULL
		};
		if ((ret = rrd_update(3, (char **) /* WEAK */ argv))) {
			sensorLog(LOG_ERR, "Error updating RRD file: %s: %s",
				  sensord_args.rrdFile, rrd_get_error());
		}
	}
	sensorLog(LOG_DEBUG, "sensor rrd updated");

	return ret;
}

int rrdCGI(void)
{
	int ret = 0, i;

	printf("#!" RRDCGI "\n\n<html>\n"
	       "<head>\n<title>sensord</title>\n</head>\n"
	       "<body>\n<h1>sensord</h1>\n");

	for (i = 0; i < ARRAY_SIZE(graphs); i++) {
		struct gr *graph = &graphs[i];

		printf("<h2>%s</h2>\n", graph->h2);
		printf("<p>\n<RRD::GRAPH %s/%s.png\n\t--imginfo '"
		       "<img src=" WWWDIR "/%%s width=%%lu height=%%lu>'"
		       "\n\t-a PNG\n\t-h 200 -w 800\n",
		       sensord_args.cgiDir, graph->image);

		printf("\t--lazy\n\t-v '%s'\n\t-t '%s'\n\t-x '%s'\n\t%s",
		       graph->axisTitle, graph->title, graph->axisDefn,
		       graph->options);
		if (!ret)
			ret = applyToFeatures(rrdCGI_DEF, graph);
		if (!ret && sensord_args.doLoad && graph->loadAvg)
			rrdCGI_DEF(graph, LOADAVG, LOAD_AVERAGE, NULL);
		if (!ret)
			ret = applyToFeatures(rrdCGI_LINE, graph);
		if (!ret && sensord_args.doLoad && graph->loadAvg)
			rrdCGI_LINE(graph, LOADAVG, LOAD_AVERAGE, NULL);
		printf (">\n</p>\n");
	}
	printf("<p>\n<small><b>sensord</b> by "
	       "<a href=\"mailto:merlin@merlin.org\">Merlin Hughes</a>"
	       ", all credit to the "
	       "<a href=\"http://www.lm-sensors.org/\">lm_sensors</a> "
	       "crew.</small>\n</p>\n");

	printf("</body>\n</html>\n");

	return ret;
}
