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
#include <getopt.h>
#include <syslog.h>

#include "args.h"
#include "sensord.h"
#include "lib/error.h"
#include "version.h"

struct sensord_arguments sensord_args = {
 	.pidFile = "/var/run/sensord.pid",
 	.scanTime = 60,
 	.logTime = 30 * 60,
 	.rrdTime = 5 * 60,
 	.syslogFacility = LOG_DAEMON,
};

static int parseTime(char *arg)
{
	char *end;
	int value = strtoul(arg, &end, 10);
	if ((end > arg) && (*end == 's')) {
		++ end;
	} else if ((end > arg) && (*end == 'm')) {
		value *= 60;
		++ end;
	} else if ((end > arg) && (*end == 'h')) {
		value *= 60 * 60;
		++ end;
	}
	if ((end == arg) || *end) {
		fprintf(stderr, "Error parsing time value `%s'.\n", arg);
		return -1;
	}
	return value;
}

static struct {
	const char *name;
	int id;
} facilities[] = {
	{ "local0", LOG_LOCAL0 },
	{ "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 },
	{ "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 },
	{ "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 },
	{ "local7", LOG_LOCAL7 },
	{ "daemon", LOG_DAEMON },
	{ "user", LOG_USER },
	{ NULL, 0 }
};

static int parseFacility(char *arg)
{
	int i = 0;
	while (facilities[i].name && strcasecmp(arg, facilities[i].name))
		++ i;
	if (!facilities[i].name) {
		fprintf(stderr, "Error parsing facility value `%s'.\n", arg);
		return -1;
	}
	return facilities[i].id;
}

static const char *daemonSyntax =
	"  -i, --interval <time>     -- interval between scanning alarms (default 60s)\n"
	"  -l, --log-interval <time> -- interval between logging sensors (default 30m)\n"
	"  -t, --rrd-interval <time> -- interval between updating RRD file (default 5m)\n"
	"  -T, --rrd-no-average      -- switch RRD in non-average mode\n"
	"  -r, --rrd-file <file>     -- RRD file (default <none>)\n"
	"  -c, --config-file <file>  -- configuration file\n"
	"  -p, --pid-file <file>     -- PID file (default /var/run/sensord.pid)\n"
	"  -f, --syslog-facility <f> -- syslog facility to use (default local4)\n"
	"  -g, --rrd-cgi <img-dir>   -- output an RRD CGI script and exit\n"
	"  -a, --load-average        -- include load average in RRD file\n"
	"  -d, --debug               -- display some debug information\n"
	"  -v, --version             -- display version and exit\n"
	"  -h, --help                -- display help and exit\n"
	"\n"
	"Specify a value of 0 for any interval to disable that operation;\n"
	"for example, specify --log-interval 0 to only scan for alarms."
	"\n"
	"Specify the filename `-' to read the config file from stdin.\n"
	"\n"
	"If no chips are specified, all chip info will be printed.\n"
	"\n"
	"If unspecified, no RRD (round robin database) is used. If specified and the\n"
	"file does not exist, it will be created. For RRD updates to be successful,\n"
	"the RRD file configuration must EXACTLY match the sensors that are used. If\n"
	"your configuration changes, delete the old RRD file and restart sensord.\n";

static const char *shortOptions = "i:l:t:Tf:r:c:p:advhg:";

static const struct option longOptions[] = {
	{ "interval", required_argument, NULL, 'i' },
	{ "log-interval", required_argument, NULL, 'l' },
	{ "rrd-interval", required_argument, NULL, 't' },
	{ "rrd-no-average", no_argument, NULL, 'T' },
	{ "syslog-facility", required_argument, NULL, 'f' },
	{ "rrd-file", required_argument, NULL, 'r' },
	{ "config-file", required_argument, NULL, 'c' },
	{ "pid-file", required_argument, NULL, 'p' },
	{ "rrd-cgi", required_argument, NULL, 'g' },
	{ "load-average", no_argument, NULL, 'a' },
	{ "debug", no_argument, NULL, 'd' },
	{ "version", no_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, 0, NULL, 0 }
};

int parseArgs(int argc, char **argv)
{
	int c;

	sensord_args.isDaemon = (argv[0][strlen (argv[0]) - 1] == 'd');
	if (!sensord_args.isDaemon) {
		fprintf(stderr, "Sensord no longer runs as an commandline"
			" application.\n");
		return -1;
  	}

	while ((c = getopt_long(argc, argv, shortOptions, longOptions, NULL))
	       != EOF) {
		switch(c) {
		case 'i':
			if ((sensord_args.scanTime = parseTime(optarg)) < 0)
				return -1;
			break;
		case 'l':
			if ((sensord_args.logTime = parseTime(optarg)) < 0)
				return -1;
			break;
		case 't':
			if ((sensord_args.rrdTime = parseTime(optarg)) < 0)
				return -1;
			break;
		case 'T':
			sensord_args.rrdNoAverage = 1;
			break;
		case 'f':
			sensord_args.syslogFacility = parseFacility(optarg);
			if (sensord_args.syslogFacility < 0)
				return -1;
			break;
		case 'a':
			sensord_args.doLoad = 1;
			break;
		case 'c':
			sensord_args.cfgFile = optarg;
			break;
		case 'p':
			sensord_args.pidFile = optarg;
			break;
		case 'r':
			sensord_args.rrdFile = optarg;
			break;
		case 'd':
			sensord_args.debug = 1;
			break;
		case 'g':
			sensord_args.doCGI = 1;
			sensord_args.cgiDir = optarg;
			break;
		case 'v':
			printf("sensord version %s\n", LM_VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			printf("Syntax: %s {options} {chips}\n%s", argv[0],
			       daemonSyntax);
			exit(EXIT_SUCCESS);
		case ':':
		case '?':
			printf("Try `%s --help' for more information.\n",
			       argv[0]);
			return -1;
		default:
			fprintf(stderr,
				"Internal error while parsing options.\n");
			return -1;
		}
	}

	if (sensord_args.doCGI && !sensord_args.rrdFile) {
		fprintf(stderr,
			"Error: Incompatible --rrd-cgi without --rrd-file.\n");
		return -1;
	}

	if (sensord_args.rrdFile && !sensord_args.rrdTime) {
		fprintf(stderr,
			"Error: Incompatible --rrd-file without --rrd-interval.\n");
		return -1;
	}

	if (!sensord_args.logTime && !sensord_args.scanTime &&
	    !sensord_args.rrdFile) {
		fprintf(stderr,
			"Error: No logging, alarm or RRD scanning.\n");
		return -1;
	}

	return 0;
}

int parseChips(int argc, char **argv)
{
	int i, n = argc - optind, err;

	if (n == 0) {
		sensord_args.chipNames[0].prefix =
			SENSORS_CHIP_NAME_PREFIX_ANY;
		sensord_args.chipNames[0].bus.type = SENSORS_BUS_TYPE_ANY;
		sensord_args.chipNames[0].bus.nr = SENSORS_BUS_NR_ANY;
		sensord_args.chipNames[0].addr = SENSORS_CHIP_NAME_ADDR_ANY;
		sensord_args.numChipNames = 1;

		return 0;
	}

	if (n > MAX_CHIP_NAMES) {
		fprintf(stderr, "Too many chip names.\n");
		return -1;
	}
	for (i = 0; i < n; ++i) {
		char *arg = argv[optind + i];

		err = sensors_parse_chip_name(arg, sensord_args.chipNames + i);
		if (err) {
			fprintf(stderr,	"Invalid chip name `%s': %s\n", arg,
				sensors_strerror(err));
			for (i--; i >= 0; i--)
				sensors_free_chip_name(sensord_args.chipNames + i);
			return -1;
		}
	}
	sensord_args.numChipNames = n;

	return 0;
}

void freeChips()
{
	int i;

	for (i = 0; i < sensord_args.numChipNames; i++)
		sensors_free_chip_name(sensord_args.chipNames + i);
}
