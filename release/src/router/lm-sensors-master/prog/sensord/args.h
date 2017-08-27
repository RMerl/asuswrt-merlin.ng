#ifndef SENSORD_ARGS_H
#define SENSORD_ARGS_H

#include <lib/sensors.h>

#define MAX_CHIP_NAMES 32

struct sensord_arguments {
	int isDaemon;
	const char *cfgFile;
	const char *pidFile;
	const char *rrdFile;
	const char *cgiDir;
	int scanTime;
	int logTime;
	int rrdTime;
	int rrdNoAverage;
	int syslogFacility;
	int doScan;
	int doSet;
	int doCGI;
	int doLoad;
	int debug;
	sensors_chip_name chipNames[MAX_CHIP_NAMES];
	int numChipNames;
};

extern struct sensord_arguments sensord_args;

#endif	/* SENSORD_ARGS_H */
