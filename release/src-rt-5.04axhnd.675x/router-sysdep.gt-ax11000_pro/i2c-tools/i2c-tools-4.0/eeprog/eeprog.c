/***************************************************************************
    copyright            : (C) by 2002-2003 Stefano Barbato
    email                : stefano@codesink.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "24cXX.h"

#define VERSION 	"0.7.5"

#define ENV_DEV		"EEPROG_DEV"
#define ENV_I2C_ADDR	"EEPROG_I2C_ADDR"

int g_quiet;

#define usage_if(a) do { do_usage_if( a , __LINE__); } while(0);
void do_usage_if(int b, int line)
{
	static const char *eeprog_usage =
"eeprog " VERSION ", a 24Cxx EEPROM reader/writer\n"
"Copyright (c) 2003 by Stefano Barbato - All rights reserved.\n"
"Usage: eeprog [-fqxdh] [-16|-8] [ -r addr[:count] | -w addr ]  /dev/i2c-N  i2c-address\n" 
"\n"
"  Address modes:\n"
"	-8		Use 8bit address mode for 24c0x...24C16 [default]\n"
"	-16		Use 16bit address mode for 24c32...24C256\n"
"  Actions:\n"
"	-r addr[:count]	Read [count] (1 if omitted) bytes from [addr]\n" 
"			and print them to the standard output\n" 
"	-w addr		Write input (stdin) at address [addr] of the EEPROM\n"
"	-h		Print this help\n"
"  Options:\n"
"	-x		Set hex output mode\n" 
"	-d		Dummy mode, display what *would* have been done\n" 
"	-f		Disable warnings and don't ask confirmation\n"
"	-q		Quiet mode\n"
"\n"
"The following environment variables could be set instead of the command\n"
"line arguments:\n"
"	EEPROG_DEV		device name(/dev/i2c-N)\n"
"	EEPROG_I2C_ADDR		i2c-address\n"
"\n"
"	Examples\n"
"	1- read 64 bytes from the EEPROM at address 0x54 on bus 0 starting\n"
"	   at address 123 (decimal)\n"
"		eeprog /dev/i2c-0 0x54 -r 123:64\n"
"	2- prints the hex codes of the first 32 bytes read from bus 1\n"
"	   at address 0x22\n"
"		eeprog /dev/i2c-1 0x51 -x -r 0x22:0x20\n"
"	3- write the current timestamp at address 0x200 of the EEPROM on\n"
"	   bus 0 at address 0x33\n"
"		date | eeprog /dev/i2c-0 0x33 -w 0x200\n";

	if(!b)
		return;
	fprintf(stderr, "%s\n[line %d]\n", eeprog_usage, line);
	exit(1);
}


#define die_if(a, msg) do { do_die_if( a , msg, __LINE__); } while(0);
void do_die_if(int b, char* msg, int line)
{
	if(!b)
		return;
	fprintf(stderr, "Error at line %d: %s\n", line, msg);
	//fprintf(stderr, "	sysmsg: %s\n", strerror(errno));
	exit(1);
}

#define print_info(args...) do { if(!g_quiet) fprintf(stderr, args); } while(0);

void parse_arg(char *arg, int* paddr, int *psize)
{
	char *end;
	*paddr = strtoul(arg, &end, 0);
	if(*end == ':')
		*psize = strtoul(++end, 0, 0);
}

int confirm_action()
{
	fprintf(stderr, 
	"\n"
	"____________________________WARNING____________________________\n"
	"Erroneously writing to a system EEPROM (like DIMM SPD modules)\n"
	"can break your system.  It will NOT boot anymore so you'll not\n"
	"be able to fix it.\n"
	"\n"
	"Reading from 8bit EEPROMs (like that in your DIMM) without using\n"
	"the -8 switch can also UNEXPECTEDLY write to them, so be sure to\n"
	"use the -8 command param when required.\n"
	"\n"
	"Use -f to disable this warning message\n"
	"\n"
	"Press ENTER to continue or hit CTRL-C to exit\n"
	"\n"
	);
	getchar();
	return 1; 
}


int read_from_eeprom(struct eeprom *e, int addr, int size, int hex)
{

	int ch, i;
	// hex print out
	die_if((ch = eeprom_read_byte(e, addr)) < 0, "read error");
	i = 1;
	if(hex)
		printf("\n %.4x|  %.2x ", addr, ch);
	else
		putchar(ch);
	while(--size)
	{
		die_if((ch = eeprom_read_current_byte(e)) < 0, "read error");
		if(hex)
		{
			addr++;
			if( (i % 16) == 0 ) 
				printf("\n %.4x|  ", addr);
			else if( (i % 8) == 0 ) 
				printf("  ");
			i++;
			printf("%.2x ", ch);
		} else {
			putchar(ch);
		}
	}
	if(hex)
		printf("\n\n");
	fflush(stdout);
	return 0;
}

int write_to_eeprom(struct eeprom *e, int addr)
{
	int c;
	while((c = getchar()) != EOF)
	{
		print_info(".");
		fflush(stdout);
		die_if(eeprom_write_byte(e, addr++, c), "write error");
	}
	print_info("\n\n");
	return 0;
}

int main(int argc, char** argv)
{
	struct eeprom e;
	int ret, op, i2c_addr, memaddr, size, want_hex, dummy, force, sixteen;
	char *device, *arg = 0, *i2c_addr_s;
	struct stat st;
	int eeprom_type = 0;

	op = want_hex = dummy = force = sixteen = 0;
	g_quiet = 0;

	while((ret = getopt(argc, argv, "1:8fr:qhw:xd")) != -1)
	{
		switch(ret)
		{
		case '1':
			usage_if(*optarg != '6' || strlen(optarg) != 1);
			die_if(eeprom_type, "EEPROM type switch (-8 or -16) used twice");
			eeprom_type = EEPROM_TYPE_16BIT_ADDR;	
			break;
		case 'x':
			want_hex++;
			break;
		case 'd':
			dummy++;
			break;
		case '8':
			die_if(eeprom_type, "EEPROM type switch (-8 or -16) used twice");
			eeprom_type = EEPROM_TYPE_8BIT_ADDR;
			break;
		case 'f':
			force++;
			break;
		case 'q':
			g_quiet++;
			break;
		case 'h':
			usage_if(1);
			break;
		default:
			die_if(op != 0, "Both read and write requested"); 
			arg = optarg;
			op = ret;
		}
	}
	if(!eeprom_type)
		eeprom_type = EEPROM_TYPE_8BIT_ADDR; // default

	usage_if(op == 0); // no switches 
	// set device and i2c_addr reading from cmdline or env
	device = i2c_addr_s = 0;
	switch(argc - optind)
	{
	case 0:
		device = getenv(ENV_DEV);
		i2c_addr_s = getenv(ENV_I2C_ADDR);
		break;
	case 1:
		if(stat(argv[optind], &st) != -1)
		{
			device = argv[optind];
			i2c_addr_s = getenv(ENV_I2C_ADDR);
		} else {
			device = getenv(ENV_DEV);
			i2c_addr_s = argv[optind];
		}
		break;
	case 2:
		device = argv[optind++];
		i2c_addr_s = argv[optind];
		break;
	default:
		usage_if(1);
	}
	usage_if(!device || !i2c_addr_s);
	i2c_addr = strtoul(i2c_addr_s, 0, 0);

	print_info("eeprog %s, a 24Cxx EEPROM reader/writer\n", VERSION);
	print_info("Copyright (c) 2003 by Stefano Barbato - All rights reserved.\n");
	print_info("  Bus: %s, Address: 0x%x, Mode: %dbit\n", 
			device, i2c_addr, 
			(eeprom_type == EEPROM_TYPE_8BIT_ADDR ? 8 : 16) );
	if(dummy)
	{
		fprintf(stderr, "Dummy mode selected, nothing done.\n");
		return 0;
	}
	die_if(eeprom_open(device, i2c_addr, eeprom_type, &e) < 0, 
			"unable to open eeprom device file (check that the file exists and that it's readable)");
	switch(op)
	{
	case 'r':
		if(force == 0)
			confirm_action();
		size = 1; // default
		parse_arg(arg, &memaddr, &size);
		print_info("  Reading %d bytes from 0x%x\n", size, memaddr);
		read_from_eeprom(&e, memaddr, size, want_hex);
		break;
	case 'w':
		if(force == 0)
			confirm_action();
		parse_arg(arg, &memaddr, &size);
		print_info("  Writing stdin starting at address 0x%x\n", 
			memaddr);
		write_to_eeprom(&e, memaddr);
		break;
	default:
		usage_if(1);
		exit(1);
	}
	eeprom_close(&e);

	return 0;
}

