/*
    isadump.c - isadump, a user-space program to dump ISA registers
    Copyright (C) 2000  Frodo Looijaard <frodol@dds.nl>, and
                        Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2011  Jean Delvare <jdelvare@suse.de>

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

/*
	Typical usage:
	isadump 0x295 0x296		Basic winbond dump using address/data registers
	isadump 0x295 0x296 2		Winbond dump, bank 2
	isadump 0x2e 0x2f 0x09		Super-I/O, logical device 9
	isadump -f 0x5000		Flat address space dump like for Via 686a
	isadump -f 0xecf0 0x10 1	PC87366, temperature channel 2
*/

#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "superio.h"

#ifdef __powerpc__
unsigned long isa_io_base = 0; /* XXX for now */
#endif /* __powerpc__ */

static void help(void)
{
	fprintf(stderr,
	        "Syntax for I2C-like access:\n"
	        "  isadump [OPTIONS] [-k V1,V2...] ADDRREG DATAREG [BANK [BANKREG]]\n"
	        "Syntax for flat address space:\n"
	        "  isadump -f [OPTIONS] ADDRESS [RANGE [BANK [BANKREG]]]\n"
		"Options:\n"
		"  -k	Super-I/O configuration access key\n"
		"  -f	Enable flat address space mode\n"
		"  -y	Assume affirmative answer to all questions\n"
		"  -W	Read and display word (16-bit) values\n"
		"  -L	Read and display long (32-bit) values\n");
}

static int default_bankreg(int flat, int addrreg, int datareg)
{
	if (flat) {
		return 0x09; /* Works for National Semiconductor
		                Super-IO chips */
	}

	if ((addrreg == 0x2e && datareg == 0x2f)
	 || (addrreg == 0x4e && datareg == 0x4f)) {
		return 0x07; /* Works for all Super-I/O chips */
	}
	
	return 0x4e; /* Works for Winbond ISA chips, default */
}

static int set_bank(int flat, int addrreg, int datareg, int bank, int bankreg)
{
	int oldbank;

	if (flat) {
		oldbank = inb(addrreg+bankreg);
		outb(bank, addrreg+bankreg);
	} else {
		outb(bankreg, addrreg);
		oldbank = inb(datareg);
		outb(bank, datareg);
	}

	return oldbank;
}

int main(int argc, char *argv[])
{
	int addrreg;        /* address in flat mode */
	int datareg = 0;    /* unused in flat mode */
	int range = 256;    /* can be changed only in flat mode */
	int bank = -1;      /* -1 means no bank operation */
	int bankreg;
	int oldbank = 0;
	int i, j;
	unsigned long res;
	int flags = 0;
	int flat = 0, yes = 0, width = 1;
	char *end;
	unsigned char enter_key[SUPERIO_MAX_KEY+1];

	enter_key[0] = 0;

	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {
		case 'f': flat = 1; break;
		case 'y': yes = 1; break;
		case 'k':
			if (2+flags >= argc
			 || superio_parse_key(enter_key, argv[2+flags]) < 0) {
				fprintf(stderr, "Invalid or missing key\n");
				help();
				exit(1);
			}
			flags++;
			break;
		case 'W': width = 2; break;
		case 'L': width = 4; break;
		default:
			fprintf(stderr, "Warning: Unsupported flag "
				"\"-%c\"!\n", argv[1+flags][1]);
			help();
			exit(1);
		}
		flags++;
	}

	/* key is never needed in flat mode */
	if (flat && enter_key[0]) {
		fprintf(stderr, "Error: Cannot use key in flat mode\n");
		exit(1);
	}

	/* verify that the argument count is correct */
	if ((!flat && argc < 1+flags+2)
	 || (flat && argc < 1+flags+1)) {
		help();
		exit(1);
	}

	addrreg = strtol(argv[1+flags], &end, 0);
	if (*end) {
		fprintf(stderr, "Error: Invalid address!\n");
		help();
		exit(1);
	}
	if (addrreg < 0 || addrreg > (flat?0xffff:0x3fff)) {
		fprintf(stderr, "Error: Address out of range "
		        "(0x0000-0x%04x)!\n", flat?0xffff:0x3fff);
		help();
		exit(1);
	}

	if (flat) {
		if (1+flags+1 < argc) {
			range = strtol(argv[1+flags+1], &end, 0);
			if (*end || range <= 0 || range > 0x100
			 || range & 0xf) {
				fprintf(stderr, "Error: Invalid range!\n"
				        "Hint: Must be a multiple of 16 no "
				        "greater than 256.\n");
				help();
				exit(1);
			}
		} else {
			addrreg &= 0xff00; /* Force alignment */
		}
	} else {
		datareg = strtol(argv[1+flags+1], &end, 0);
		if (*end) {
			fprintf(stderr, "Error: Invalid data register!\n");
			help();
			exit(1);
		}
		if (datareg < 0 || datareg > 0x3fff) {
			fprintf(stderr, "Error: Data register out of range "
			        "(0x0000-0x3fff)!\n");
			help();
			exit(1);
		}
	}

	bankreg = default_bankreg(flat, addrreg, datareg);

	if (1+flags+2 < argc) {
		bank = strtol(argv[1+flags+2], &end, 0);
		if (*end) {
			fprintf(stderr, "Error: Invalid bank number!\n");
			help();
			exit(1);
		}
		if ((bank < 0) || (bank > 31)) {
			fprintf(stderr, "Error: bank out of range (0-31)!\n");
			help();
			exit(1);
		}

		if (1+flags+3 < argc) {
			bankreg = strtol(argv[1+flags+3], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: Invalid bank "
				        "register!\n");
				help();
				exit(1);
			}
			if (bankreg < 0 || bankreg >= range) {
				fprintf(stderr, "Error: bank out of range "
				        "(0x00-0x%02x)!\n", range-1);
				help();
				exit(1);
			}
		}
	}

	if (geteuid()) {
		fprintf(stderr, "Error: Can only be run as root (or make it "
		        "suid root)\n");
		exit(1);
	}

	if (!yes) {
		fprintf(stderr, "WARNING! Running this program can cause "
		        "system crashes, data loss and worse!\n");

		if (flat)
			fprintf(stderr, "I will probe address range 0x%x to "
			        "0x%x.\n", addrreg, addrreg + range - 1);
		else
			fprintf(stderr, "I will probe address register 0x%x "
			        "and data register 0x%x.\n", addrreg, datareg);

		if (bank>=0) 	
			fprintf(stderr, "Probing bank %d using bank register "
			        "0x%02x.\n", bank, bankreg);

		fprintf(stderr, "Continue? [Y/n] ");
		fflush(stderr);
		if (!user_ack(1)) {
			fprintf(stderr, "Aborting on user request.\n");
			exit(0);
		}
	}

#ifndef __powerpc__
	if ((datareg < 0x400) && (addrreg < 0x400) && !flat) {
		if (ioperm(datareg, 1, 1)) {
			fprintf(stderr, "Error: Could not ioperm() data "
			        "register!\n");
			exit(1);
		}
		if (ioperm(addrreg, 1, 1)) {
			fprintf(stderr, "Error: Could not ioperm() address "
			        "register!\n");
			exit(1);
		}
	} else {
		if (iopl(3)) {
			fprintf(stderr, "Error: Could not do iopl(3)!\n");
			exit(1);
		}
	}
#endif

	/* Enter Super-I/O configuration mode */
	if (enter_key[0])
		superio_write_key(addrreg, enter_key);

	if (bank >= 0)
		oldbank = set_bank(flat, addrreg, datareg, bank, bankreg);

	/* print column headers */
	printf("%*s", flat ? 5 : 3, "");
	for (j = 0; j < 16; j += width)
		printf(" %*x", width * 2, j);
	printf("\n");

	for (i = 0; i < range; i += 16) {
		if (flat)
			printf("%04x: ", addrreg + i);
		else
			printf("%02x: ", i);

		/* It was noticed that Winbond Super-I/O chips
		   would leave the configuration mode after
		   an arbitrary number of register reads,
		   causing any subsequent read attempt to
		   silently fail. Repeating the key every 16 reads
		   prevents that. */
		if (enter_key[0])
			superio_write_key(addrreg, enter_key);

		for (j = 0; j < 16; j += width) {
			fflush(stdout);
			if (flat) {
				res = inx(addrreg + i + j, width);
			} else {	
				outb(i+j, addrreg);
				if (i+j == 0 && inb(addrreg) == 0x80) {
					/* Bit 7 appears to be a busy flag */
					range = 128;
				}
				res = inx(datareg, width);
			}
			printf("%0*lx ", width * 2, res);
		}
		printf("\n");
	}

	/* Restore the original bank value */
	if (bank >= 0)
		set_bank(flat, addrreg, datareg, oldbank, bankreg);

	/* Exit Super-I/O configuration mode */
	if (enter_key[0])
		superio_reset(addrreg, datareg);

	exit(0);
}
