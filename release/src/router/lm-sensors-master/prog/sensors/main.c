/*
    main.c - Part of sensors, a user-space program for hardware monitoring
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
    Copyright (C) 2007-2012   Jean Delvare <jdelvare@suse.de>

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
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <langinfo.h>

#ifndef __UCLIBC__
#include <iconv.h>
#define HAVE_ICONV
#endif

#include "lib/sensors.h"
#include "lib/error.h"
#include "main.h"
#include "chips.h"
#include "version.h"

#define PROGRAM			"sensors"
#define VERSION			LM_VERSION

static int do_sets, do_raw, hide_adapter;

int fahrenheit;
char degstr[5]; /* store the correct string to print degrees */

static void print_short_help(void)
{
	printf("Try `%s -h' for more information\n", PROGRAM);
}

static void print_long_help(void)
{
	printf("Usage: %s [OPTION]... [CHIP]...\n", PROGRAM);
	puts("  -c, --config-file     Specify a config file\n"
	     "  -h, --help            Display this help text\n"
	     "  -s, --set             Execute `set' statements (root only)\n"
	     "  -f, --fahrenheit      Show temperatures in degrees fahrenheit\n"
	     "  -A, --no-adapter      Do not show adapter for each chip\n"
	     "      --bus-list        Generate bus statements for sensors.conf\n"
	     "  -u                    Raw output\n"
	     "  -v, --version         Display the program version\n"
	     "\n"
	     "Use `-' after `-c' to read the config file from stdin.\n"
	     "If no chips are specified, all chip info will be printed.\n"
	     "Example chip names:\n"
	     "\tlm78-i2c-0-2d\t*-i2c-0-2d\n"
	     "\tlm78-i2c-0-*\t*-i2c-0-*\n"
	     "\tlm78-i2c-*-2d\t*-i2c-*-2d\n"
	     "\tlm78-i2c-*-*\t*-i2c-*-*\n"
	     "\tlm78-isa-0290\t*-isa-0290\n"
	     "\tlm78-isa-*\t*-isa-*\n"
	     "\tlm78-*");
}

static void print_version(void)
{
	printf("%s version %s with libsensors version %s\n", PROGRAM, VERSION,
	       libsensors_version);
}

/* Return 0 on success, and an exit error code otherwise */
static int read_config_file(const char *config_file_name)
{
	FILE *config_file;
	int err;

	if (config_file_name) {
		if (!strcmp(config_file_name, "-"))
			config_file = stdin;
		else
			config_file = fopen(config_file_name, "r");

		if (!config_file) {
			fprintf(stderr, "Could not open config file\n");
			perror(config_file_name);
			return 1;
		}
	} else {
		/* Use libsensors default */
		config_file = NULL;
	}

	err = sensors_init(config_file);
	if (err) {
		fprintf(stderr, "sensors_init: %s\n", sensors_strerror(err));
		if (config_file)
			fclose(config_file);
		return 1;
	}

	if (config_file && fclose(config_file) == EOF)
		perror(config_file_name);

	return 0;
}

static void set_degstr(void)
{
	const char *deg_default_text[2] = { " C", " F" };

#ifdef HAVE_ICONV
	/* Size hardcoded for better performance.
	   Don't forget to count the trailing \0! */
	size_t deg_latin1_size = 3;
	char deg_latin1_text[2][3] = { "\260C", "\260F" };
	char *deg_latin1_ptr = deg_latin1_text[fahrenheit];
	size_t nconv;
	size_t degstr_size = sizeof(degstr);
	char *degstr_ptr = degstr;

	iconv_t cd = iconv_open(nl_langinfo(CODESET), "ISO-8859-1");
	if (cd != (iconv_t) -1) {
		nconv = iconv(cd, &deg_latin1_ptr, &deg_latin1_size,
			      &degstr_ptr, &degstr_size);
		iconv_close(cd);

		if (nconv != (size_t) -1)
			return;
	}
#endif /* HAVE_ICONV */

	/* There was an error during the conversion, use the default text */
	strcpy(degstr, deg_default_text[fahrenheit]);
}

static const char *sprintf_chip_name(const sensors_chip_name *name)
{
#define BUF_SIZE 200
	static char buf[BUF_SIZE];

	if (sensors_snprintf_chip_name(buf, BUF_SIZE, name) < 0)
		return NULL;
	return buf;
}

static void do_a_print(const sensors_chip_name *name)
{
	printf("%s\n", sprintf_chip_name(name));
	if (!hide_adapter) {
		const char *adap = sensors_get_adapter_name(&name->bus);
		if (adap)
			printf("Adapter: %s\n", adap);
		else
			fprintf(stderr, "Can't get adapter name\n");
	}
	if (do_raw)
		print_chip_raw(name);
	else
		print_chip(name);
	printf("\n");
}

/* returns 1 on error */
static int do_a_set(const sensors_chip_name *name)
{
	int err;

	if ((err = sensors_do_chip_sets(name))) {
		if (err == -SENSORS_ERR_KERNEL) {
			fprintf(stderr, "%s: %s\n",
				sprintf_chip_name(name),
				sensors_strerror(err));
			fprintf(stderr, "Run as root?\n");
			return 1;
		} else if (err == -SENSORS_ERR_ACCESS_W) {
			fprintf(stderr,
				"%s: At least one \"set\" statement failed\n",
				sprintf_chip_name(name));
		} else {
			fprintf(stderr, "%s: %s\n", sprintf_chip_name(name),
				sensors_strerror(err));
		}
	}
	return 0;
}

/* returns number of chips found */
static int do_the_real_work(const sensors_chip_name *match, int *err)
{
	const sensors_chip_name *chip;
	int chip_nr;
	int cnt = 0;

	chip_nr = 0;
	while ((chip = sensors_get_detected_chips(match, &chip_nr))) {
		if (do_sets) {
			if (do_a_set(chip))
				*err = 1;
		} else
			do_a_print(chip);
		cnt++;
	}
	return cnt;
}

/* List the buses in a format suitable for sensors.conf. We only list
   bus types for which bus statements are actually useful and supported.
   Known bug: i2c buses with number >= 32 or 64 could be listed several
   times. Very unlikely to ever happen, though. */
static void print_bus_list(void)
{
	const sensors_chip_name *chip;
	int chip_nr;
	unsigned long seen_i2c = 0;

	chip_nr = 0;
	while ((chip = sensors_get_detected_chips(NULL, &chip_nr))) {
		switch (chip->bus.type) {
		case SENSORS_BUS_TYPE_I2C:
			if (chip->bus.nr < (int)sizeof(unsigned long) * 8) {
				if (seen_i2c & (1 << chip->bus.nr))
					break;
				seen_i2c |= 1 << chip->bus.nr;
			}
			printf("bus \"i2c-%d\" \"%s\"\n", chip->bus.nr,
			       sensors_get_adapter_name(&chip->bus));
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int c, i, err, do_bus_list;
	const char *config_file_name = NULL;

	struct option long_opts[] =  {
		{ "help", no_argument, NULL, 'h' },
		{ "set", no_argument, NULL, 's' },
		{ "version", no_argument, NULL, 'v'},
		{ "fahrenheit", no_argument, NULL, 'f' },
		{ "no-adapter", no_argument, NULL, 'A' },
		{ "config-file", required_argument, NULL, 'c' },
		{ "bus-list", no_argument, NULL, 'B' },
		{ 0, 0, 0, 0 }
	};

	setlocale(LC_CTYPE, "");

	do_raw = 0;
	do_sets = 0;
	do_bus_list = 0;
	hide_adapter = 0;
	while (1) {
		c = getopt_long(argc, argv, "hsvfAc:u", long_opts, NULL);
		if (c == EOF)
			break;
		switch(c) {
		case ':':
		case '?':
			print_short_help();
			exit(1);
		case 'h':
			print_long_help();
			exit(0);
		case 'v':
			print_version();
			exit(0);
		case 'c':
			config_file_name = optarg;
			break;
		case 's':
			do_sets = 1;
			break;
		case 'f':
			fahrenheit = 1;
			break;
		case 'A':
			hide_adapter = 1;
			break;
		case 'u':
			do_raw = 1;
			break;
		case 'B':
			do_bus_list = 1;
			break;
		default:
			fprintf(stderr,
				"Internal error while parsing options!\n");
			exit(1);
		}
	}

	err = read_config_file(config_file_name);
	if (err)
		exit(err);

	/* build the degrees string */
	set_degstr();

	if (do_bus_list) {
		print_bus_list();
	} else if (optind == argc) { /* No chip name on command line */
		if (!do_the_real_work(NULL, &err)) {
			fprintf(stderr,
				"No sensors found!\n"
				"Make sure you loaded all the kernel drivers you need.\n"
				"Try sensors-detect to find out which these are.\n");
			err = 1;
		}
	} else {
		int cnt = 0;
		sensors_chip_name chip;

		for (i = optind; i < argc; i++) {
			if (sensors_parse_chip_name(argv[i], &chip)) {
				fprintf(stderr,
					"Parse error in chip name `%s'\n",
					argv[i]);
				print_short_help();
				err = 1;
				goto exit;
			}
			cnt += do_the_real_work(&chip, &err);
			sensors_free_chip_name(&chip);
		}

		if (!cnt) {
			fprintf(stderr, "Specified sensor(s) not found!\n");
			err = 1;
		}
	}

exit:
	sensors_cleanup();
	exit(err);
}
