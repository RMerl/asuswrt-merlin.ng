/*
    i2cget.c - A user-space program to read an I2C register.
    Copyright (C) 2005-2012  Jean Delvare <jdelvare@suse.de>

    Based on i2cset.c:
    Copyright (C) 2001-2003  Frodo Looijaard <frodol@dds.nl>, and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2005  Jean Delvare

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
#include <linux/i2c-dev.h>
#include "i2cbusses.h"
#include "util.h"
#include "../version.h"

static void help(void) __attribute__ ((noreturn));

static void help(void)
{
	fprintf(stderr,
		"Usage: i2cget [-f] [-y] I2CBUS CHIP-ADDRESS [DATA-ADDRESS [MODE] [LENGTH]]\n"
		"  I2CBUS is an integer or an I2C bus name\n"
		"  ADDRESS is an integer (0x03 - 0x77)\n"
		"  MODE is one of:\n"
		"	 b (read byte data, default)\n"
		"	 w (read word data)\n"
		"	 c (write byte/read byte)\n"
		"	 i (read I2C block data)\n"
		"	 Append p for SMBus PEC\n"
		"  LENGTH is length for block data reads\n");
	exit(1);
}

static int check_funcs(int file, int size, int daddress, int pec)
{
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	switch (size) {
	case I2C_SMBUS_BYTE:
		if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus receive byte");
			return -1;
		}
		if (daddress >= 0
		 && !(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
			return -1;
		}
		break;

	case I2C_SMBUS_BYTE_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus read byte");
			return -1;
		}
		break;

	case I2C_SMBUS_WORD_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_READ_WORD_DATA)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus read word");
			return -1;
		}
		break;

	case I2C_SMBUS_I2C_BLOCK_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_READ_I2C_BLOCK)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus read I2C block data");
			return -1;
		}
		break;
	}

	if (pec
	 && !(funcs & (I2C_FUNC_SMBUS_PEC | I2C_FUNC_I2C))) {
		fprintf(stderr, "Warning: Adapter does "
			"not seem to support PEC\n");
	}

	return 0;
}

static int confirm(const char *filename, int address, int size, int daddress,
		   int length, int pec)
{
	int dont = 0;

	fprintf(stderr, "WARNING! This program can confuse your I2C "
		"bus, cause data loss and worse!\n");

	/* Don't let the user break his/her EEPROMs */
	if (address >= 0x50 && address <= 0x57 && pec) {
		fprintf(stderr, "STOP! EEPROMs are I2C devices, not "
			"SMBus devices. Using PEC\non I2C devices may "
			"result in unexpected results, such as\n"
			"trashing the contents of EEPROMs. We can't "
			"let you do that, sorry.\n");
		return 0;
	}

	if (size == I2C_SMBUS_BYTE && daddress >= 0 && pec) {
		fprintf(stderr, "WARNING! All I2C chips and some SMBus chips "
			"will interpret a write\nbyte command with PEC as a"
			"write byte data command, effectively writing a\n"
			"value into a register!\n");
		dont++;
	}

	fprintf(stderr, "I will read from device file %s, chip "
		"address 0x%02x, ", filename, address);
	if (daddress < 0)
		fprintf(stderr, "current data\naddress");
	else
		fprintf(stderr, "data address\n0x%02x", daddress);
		if (size == I2C_SMBUS_I2C_BLOCK_DATA)
			fprintf(stderr, ", %d bytes using read I2C block data.\n", length);
		else
			fprintf(stderr, ", using %s.\n",
					size == I2C_SMBUS_BYTE ? (daddress < 0 ?
					"read byte" : "write byte/read byte") :
					size == I2C_SMBUS_BYTE_DATA ? "read byte data" :
					"read word data");
	if (pec)
		fprintf(stderr, "PEC checking enabled.\n");

	fprintf(stderr, "Continue? [%s] ", dont ? "y/N" : "Y/n");
	fflush(stderr);
	if (!user_ack(!dont)) {
		fprintf(stderr, "Aborting on user request.\n");
		return 0;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	char *end;
	int res, i2cbus, address, size, file;
	int daddress;
	char filename[20];
	int pec = 0;
	int flags = 0;
	int force = 0, yes = 0, version = 0;
	int length;
	__u8 block_data[I2C_SMBUS_BLOCK_MAX+2];

	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {
		case 'V': version = 1; break;
		case 'f': force = 1; break;
		case 'y': yes = 1; break;
		default:
			fprintf(stderr, "Error: Unsupported option "
				"\"%s\"!\n", argv[1+flags]);
			help();
			exit(1);
		}
		flags++;
	}

	if (version) {
		fprintf(stderr, "i2cget version %s\n", VERSION);
		exit(0);
	}

	if (argc < flags + 3)
		help();

	i2cbus = lookup_i2c_bus(argv[flags+1]);
	if (i2cbus < 0)
		help();

	address = parse_i2c_address(argv[flags+2]);
	if (address < 0)
		help();

	if (argc > flags + 3) {
		size = I2C_SMBUS_BYTE_DATA;
		daddress = strtol(argv[flags+3], &end, 0);
		if (*end || daddress < 0 || daddress > 0xff) {
			fprintf(stderr, "Error: Data address invalid!\n");
			help();
		}
	} else {
		size = I2C_SMBUS_BYTE;
		daddress = -1;
	}

	if (argc > flags + 4) {
		switch (argv[flags+4][0]) {
		case 'b': size = I2C_SMBUS_BYTE_DATA; break;
		case 'w': size = I2C_SMBUS_WORD_DATA; break;
		case 'c': size = I2C_SMBUS_BYTE; break;
		case 'i': size = I2C_SMBUS_I2C_BLOCK_DATA; break;
		default:
			fprintf(stderr, "Error: Invalid mode!\n");
			help();
		}
		pec = argv[flags+4][1] == 'p';
	}

	if (argc > flags + 5) {
		if (size != I2C_SMBUS_I2C_BLOCK_DATA) {
			fprintf(stderr, "Error: Length only valid for I2C block data!\n");
			help();
		}
		length = strtol(argv[flags+5], &end, 0);
		if (*end || length < 1 || length > I2C_SMBUS_BLOCK_MAX) {
			fprintf(stderr, "Error: Length invalid!\n");
			help();
		}
	} else {
		length = I2C_SMBUS_BLOCK_MAX;
	}

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	if (file < 0
	 || check_funcs(file, size, daddress, pec)
	 || set_slave_addr(file, address, force))
		exit(1);

	if (!yes && !confirm(filename, address, size, daddress, length, pec))
		exit(0);

	if (pec && ioctl(file, I2C_PEC, 1) < 0) {
		fprintf(stderr, "Error: Could not set PEC: %s\n",
			strerror(errno));
		close(file);
		exit(1);
	}

	switch (size) {
	case I2C_SMBUS_BYTE:
		if (daddress >= 0) {
			res = i2c_smbus_write_byte(file, daddress);
			if (res < 0)
				fprintf(stderr, "Warning - write failed\n");
		}
		res = i2c_smbus_read_byte(file);
		break;
	case I2C_SMBUS_WORD_DATA:
		res = i2c_smbus_read_word_data(file, daddress);
		break;
	case I2C_SMBUS_I2C_BLOCK_DATA:
		res = i2c_smbus_read_i2c_block_data(file, daddress, length, block_data);
		break;
	default: /* I2C_SMBUS_BYTE_DATA */
		res = i2c_smbus_read_byte_data(file, daddress);
	}
	close(file);

	if (res < 0) {
		fprintf(stderr, "Error: Read failed\n");
		exit(2);
	}

	if (size == I2C_SMBUS_I2C_BLOCK_DATA) {
		int i;
		//printf("%d:", res);
		if(res > 1) {
			for (i = 1; i <= res; ++i) {
				printf("%d ", block_data[i]);
			}
		}
		else if(res == 1)
			printf("%d ", block_data[0]);
			
		printf("\n");
	} else {
		printf("0x%0*x\n", size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
	}

	exit(0);
}
