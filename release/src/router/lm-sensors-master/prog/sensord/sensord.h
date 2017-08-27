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

#include "lib/sensors.h"

#define ARRAY_SIZE(arr)	(int)(sizeof(arr) / sizeof((arr)[0]))

extern void sensorLog(int priority, const char *fmt, ...);

/* from args.c */

extern int parseArgs(int argc, char **argv);
extern int parseChips(int argc, char **argv);
extern void freeChips(void);

/* from lib.c */

extern int loadLib(const char *cfgPath);
extern int reloadLib(const char *cfgPath);
extern int unloadLib(void);

/* from sense.c */

extern int readChips(void);
extern int scanChips(void);
extern int setChips(void);
extern int rrdChips(void);

/* from rrd.c */

extern char rrdBuff[];
extern int rrdInit(void);
extern int rrdUpdate(void);
extern int rrdCGI(void);

/* from chips.c */

#define MAX_DATA 5

typedef const char *(*FormatterFN) (const double values[], int alrm,
				     int beep);

typedef const char *(*RRDFN) (const double values[]);

typedef enum {
	DataType_voltage = 0,
	DataType_rpm,
	DataType_temperature,
	DataType_other = -1
} DataType;

typedef struct {
	FormatterFN format;
	RRDFN rrd;
	DataType type;
	int alarmNumber;
	int beepNumber;
	const sensors_feature *feature;
	int dataNumbers[MAX_DATA + 1];
} FeatureDescriptor;

typedef struct {
	const sensors_chip_name *name;
	FeatureDescriptor *features;
} ChipDescriptor;

extern ChipDescriptor * knownChips;
extern int initKnownChips(void);
extern void freeKnownChips(void);
