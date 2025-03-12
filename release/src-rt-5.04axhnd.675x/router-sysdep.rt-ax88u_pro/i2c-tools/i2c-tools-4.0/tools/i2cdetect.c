/*
    i2cdetect.c - a user-space program to scan for I2C devices
    Copyright (C) 1999-2004  Frodo Looijaard <frodol@dds.nl>, and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2012  Jean Delvare <jdelvare@suse.de>

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

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "i2cbusses.h"
#include "../version.h"

#define MODE_AUTO	0
#define MODE_QUICK	1
#define MODE_READ	2
#define MODE_FUNC	3

static void help(void)
{
	fprintf(stderr,
		"Usage: i2cdetect [-y] [-a] [-q|-r] I2CBUS [FIRST LAST]\n"
		"       i2cdetect -F I2CBUS\n"
		"       i2cdetect -l\n"
		"  I2CBUS is an integer or an I2C bus name\n"
		"  If provided, FIRST and LAST limit the probing range.\n");
}

static int scan_i2c_bus(int file, int mode, unsigned long funcs,
			int first, int last)
{
	int i, j;
	int cmd, res;

	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", i);
		for(j = 0; j < 16; j++) {
			fflush(stdout);

			/* Select detection command for this address */
			switch (mode) {
			default:
				cmd = mode;
				break;
			case MODE_AUTO:
				if ((i+j >= 0x30 && i+j <= 0x37)
				 || (i+j >= 0x50 && i+j <= 0x5F))
				 	cmd = MODE_READ;
				else
					cmd = MODE_QUICK;
				break;
			}

			/* Skip unwanted addresses */
			if (i+j < first || i+j > last
			 || (cmd == MODE_READ &&
			     !(funcs & I2C_FUNC_SMBUS_READ_BYTE))
			 || (cmd == MODE_QUICK &&
			     !(funcs & I2C_FUNC_SMBUS_QUICK))) {
				printf("   ");
				continue;
			}

			/* Set slave address */
			if (ioctl(file, I2C_SLAVE, i+j) < 0) {
				if (errno == EBUSY) {
					printf("UU ");
					continue;
				} else {
					fprintf(stderr, "Error: Could not set "
						"address to 0x%02x: %s\n", i+j,
						strerror(errno));
					return -1;
				}
			}

			/* Probe this address */
			switch (cmd) {
			default: /* MODE_QUICK */
				/* This is known to corrupt the Atmel AT24RF08
				   EEPROM */
				res = i2c_smbus_write_quick(file,
				      I2C_SMBUS_WRITE);
				break;
			case MODE_READ:
				/* This is known to lock SMBus on various
				   write-only chips (mainly clock chips) */
				res = i2c_smbus_read_byte(file);
				break;
			}

			if (res < 0)
				printf("-- ");
			else
				printf("%02x ", i+j);
		}
		printf("\n");
	}

	return 0;
}

struct func
{
	long value;
	const char* name;
};

static const struct func all_func[] = {
	{ .value = I2C_FUNC_I2C,
	  .name = "I2C" },
	{ .value = I2C_FUNC_SMBUS_QUICK,
	  .name = "SMBus Quick Command" },
	{ .value = I2C_FUNC_SMBUS_WRITE_BYTE,
	  .name = "SMBus Send Byte" },
	{ .value = I2C_FUNC_SMBUS_READ_BYTE,
	  .name = "SMBus Receive Byte" },
	{ .value = I2C_FUNC_SMBUS_WRITE_BYTE_DATA,
	  .name = "SMBus Write Byte" },
	{ .value = I2C_FUNC_SMBUS_READ_BYTE_DATA,
	  .name = "SMBus Read Byte" },
	{ .value = I2C_FUNC_SMBUS_WRITE_WORD_DATA,
	  .name = "SMBus Write Word" },
	{ .value = I2C_FUNC_SMBUS_READ_WORD_DATA,
	  .name = "SMBus Read Word" },
	{ .value = I2C_FUNC_SMBUS_PROC_CALL,
	  .name = "SMBus Process Call" },
	{ .value = I2C_FUNC_SMBUS_WRITE_BLOCK_DATA,
	  .name = "SMBus Block Write" },
	{ .value = I2C_FUNC_SMBUS_READ_BLOCK_DATA,
	  .name = "SMBus Block Read" },
	{ .value = I2C_FUNC_SMBUS_BLOCK_PROC_CALL,
	  .name = "SMBus Block Process Call" },
	{ .value = I2C_FUNC_SMBUS_PEC,
	  .name = "SMBus PEC" },
	{ .value = I2C_FUNC_SMBUS_WRITE_I2C_BLOCK,
	  .name = "I2C Block Write" },
	{ .value = I2C_FUNC_SMBUS_READ_I2C_BLOCK,
	  .name = "I2C Block Read" },
	{ .value = 0, .name = "" }
};

static void print_functionality(unsigned long funcs)
{
	int i;

	for (i = 0; all_func[i].value; i++) {
		printf("%-32s %s\n", all_func[i].name,
		       (funcs & all_func[i].value) ? "yes" : "no");
	}
}

/*
 * Print the installed i2c busses. The format is those of Linux 2.4's
 * /proc/bus/i2c for historical compatibility reasons.
 */
static void print_i2c_busses(void)
{
	struct i2c_adap *adapters;
	int count;

	adapters = gather_i2c_busses();
	if (adapters == NULL) {
		fprintf(stderr, "Error: Out of memory!\n");
		return;
	}

	for (count = 0; adapters[count].name; count++) {
		printf("i2c-%d\t%-10s\t%-32s\t%s\n",
			adapters[count].nr, adapters[count].funcs,
			adapters[count].name, adapters[count].algo);
	}

	free_adapters(adapters);
}

int main(int argc, char *argv[])
{
	char *end;
	int i2cbus, file, res;
	char filename[20];
	unsigned long funcs;
	int mode = MODE_AUTO;
	int first = 0x03, last = 0x77;
	int flags = 0;
	int yes = 0, version = 0, list = 0;

	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {
		case 'V': version = 1; break;
		case 'y': yes = 1; break;
		case 'l': list = 1; break;
		case 'F':
			if (mode != MODE_AUTO && mode != MODE_FUNC) {
				fprintf(stderr, "Error: Different modes "
					"specified!\n");
				exit(1);
			}
			mode = MODE_FUNC;
			break;
		case 'r':
			if (mode == MODE_QUICK) {
				fprintf(stderr, "Error: Different modes "
					"specified!\n");
				exit(1);
			}
			mode = MODE_READ;
			break;
		case 'q':
			if (mode == MODE_READ) {
				fprintf(stderr, "Error: Different modes "
					"specified!\n");
				exit(1);
			}
			mode = MODE_QUICK;
			break;
		case 'a':
			first = 0x00;
			last = 0x7F;
			break;
		default:
			fprintf(stderr, "Error: Unsupported option "
				"\"%s\"!\n", argv[1+flags]);
			help();
			exit(1);
		}
		flags++;
	}

	if (version) {
		fprintf(stderr, "i2cdetect version %s\n", VERSION);
		exit(0);
	}

	if (list) {
		print_i2c_busses();
		exit(0);
	}

	if (argc < flags + 2) {
		fprintf(stderr, "Error: No i2c-bus specified!\n");
		help();
		exit(1);
	}
	i2cbus = lookup_i2c_bus(argv[flags+1]);
	if (i2cbus < 0) {
		help();
		exit(1);
	}

	/* read address range if present */
	if (argc == flags + 4 && mode != MODE_FUNC) {
		int tmp;

		tmp = strtol(argv[flags+2], &end, 0);
		if (*end) {
			fprintf(stderr, "Error: FIRST argment not a "
				"number!\n");
			help();
			exit(1);
		}
		if (tmp < first || tmp > last) {
			fprintf(stderr, "Error: FIRST argument out of range "
				"(0x%02x-0x%02x)!\n", first, last);
			help();
			exit(1);
		}
		first = tmp;

		tmp = strtol(argv[flags+3], &end, 0);
		if (*end) {
			fprintf(stderr, "Error: LAST argment not a "
				"number!\n");
			help();
			exit(1);
		}
		if (tmp < first || tmp > last) {
			fprintf(stderr, "Error: LAST argument out of range "
				"(0x%02x-0x%02x)!\n", first, last);
			help();
			exit(1);
		}
		last = tmp;
	} else if (argc != flags + 2) {
		help();
		exit(1);
	}

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	if (file < 0) {
		exit(1);
	}

	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		close(file);
		exit(1);
	}

	/* Special case, we only list the implemented functionalities */
	if (mode == MODE_FUNC) {
		close(file);
		printf("Functionalities implemented by %s:\n", filename);
		print_functionality(funcs);
		exit(0);
	}

	if (!(funcs & (I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_READ_BYTE))) {
		fprintf(stderr,
			"Error: Bus doesn't support detection commands\n");
		close(file);
		exit(1);
	}
	if (mode == MODE_QUICK && !(funcs & I2C_FUNC_SMBUS_QUICK)) {
		fprintf(stderr, "Error: Can't use SMBus Quick Write command "
			"on this bus\n");
		close(file);
		exit(1);
	}
	if (mode == MODE_READ && !(funcs & I2C_FUNC_SMBUS_READ_BYTE)) {
		fprintf(stderr, "Error: Can't use SMBus Receive Byte command "
			"on this bus\n");
		close(file);
		exit(1);
	}
	if (mode == MODE_AUTO) {
		if (!(funcs & I2C_FUNC_SMBUS_QUICK))
			fprintf(stderr, "Warning: Can't use SMBus Quick Write "
				"command, will skip some addresses\n");
		if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE))
			fprintf(stderr, "Warning: Can't use SMBus Receive Byte "
				"command, will skip some addresses\n");
	}

	if (!yes) {
		char s[2];

		fprintf(stderr, "WARNING! This program can confuse your I2C "
			"bus, cause data loss and worse!\n");

		fprintf(stderr, "I will probe file %s%s.\n", filename,
			mode==MODE_QUICK?" using quick write commands":
			mode==MODE_READ?" using receive byte commands":"");
		fprintf(stderr, "I will probe address range 0x%02x-0x%02x.\n",
			first, last);

		fprintf(stderr, "Continue? [Y/n] ");
		fflush(stderr);
		if (!fgets(s, 2, stdin)
		 || (s[0] != '\n' && s[0] != 'y' && s[0] != 'Y')) {
			fprintf(stderr, "Aborting on user request.\n");
			exit(0);
		}
	}

	res = scan_i2c_bus(file, mode, funcs, first, last);

	close(file);

	exit(res?1:0);
}
