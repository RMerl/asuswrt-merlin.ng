// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Robin Getz rgetz@blacfin.uclinux.org
 *
 * Heavily borrowed from the following peoples GPL'ed software:
 *  - Wolfgang Denk, DENX Software Engineering, wd@denx.de
 *       Das U-Boot
 *  - Ladislav Michl ladis@linux-mips.org
 *       A rejected patch on the U-Boot mailing list
 */

#include <common.h>
#include <exports.h>
#include "../drivers/net/smc91111.h"

#ifndef SMC91111_EEPROM_INIT
# define SMC91111_EEPROM_INIT()
#endif

#define SMC_BASE_ADDRESS CONFIG_SMC91111_BASE
#define EEPROM		0x1
#define MAC		0x2
#define UNKNOWN		0x4

void dump_reg (struct eth_device *dev);
void dump_eeprom (struct eth_device *dev);
int write_eeprom_reg (struct eth_device *dev, int value, int reg);
void copy_from_eeprom (struct eth_device *dev);
void print_MAC (struct eth_device *dev);
int read_eeprom_reg (struct eth_device *dev, int reg);
void print_macaddr (struct eth_device *dev);

int smc91111_eeprom (int argc, char * const argv[])
{
	int c, i, j, done, line, reg, value, start, what;
	char input[50];

	struct eth_device dev;
	dev.iobase = CONFIG_SMC91111_BASE;

	/* Print the ABI version */
	app_startup (argv);
	if (XF_VERSION != (int) get_version ()) {
		printf ("Expects ABI version %d\n", XF_VERSION);
		printf ("Actual U-Boot ABI version %d\n",
			(int) get_version ());
		printf ("Can't run\n\n");
		return (0);
	}

	SMC91111_EEPROM_INIT();

	if ((SMC_inw (&dev, BANK_SELECT) & 0xFF00) != 0x3300) {
		printf ("Can't find SMSC91111\n");
		return (0);
	}

	done = 0;
	what = UNKNOWN;
	printf ("\n");
	while (!done) {
		/* print the prompt */
		printf ("SMC91111> ");
		line = 0;
		i = 0;
		start = 1;
		while (!line) {
			/* Wait for a keystroke */
			while (!tstc ());

			c = getc ();
			/* Make Uppercase */
			if (c >= 'Z')
				c -= ('a' - 'A');
			/* printf(" |%02x| ",c); */

			switch (c) {
			case '\r':	/* Enter                */
			case '\n':
				input[i] = 0;
				puts ("\r\n");
				line = 1;
				break;
			case '\0':	/* nul                  */
				continue;

			case 0x03:	/* ^C - break           */
				input[0] = 0;
				i = 0;
				line = 1;
				done = 1;
				break;

			case 0x5F:
			case 0x08:	/* ^H  - backspace      */
			case 0x7F:	/* DEL - backspace      */
				if (i > 0) {
					puts ("\b \b");
					i--;
				}
				break;
			default:
				if (start) {
					if ((c == 'W') || (c == 'D')
					    || (c == 'M') || (c == 'C')
					    || (c == 'P')) {
						putc (c);
						input[i] = c;
						if (i <= 45)
							i++;
						start = 0;
					}
				} else {
					if ((c >= '0' && c <= '9')
					    || (c >= 'A' && c <= 'F')
					    || (c == 'E') || (c == 'M')
					    || (c == ' ')) {
						putc (c);
						input[i] = c;
						if (i <= 45)
							i++;
						break;
					}
				}
				break;
			}
		}

		for (; i < 49; i++)
			input[i] = 0;

		switch (input[0]) {
		case ('W'):
			/* Line should be w reg value */
			i = 0;
			reg = 0;
			value = 0;
			/* Skip to the next space or end) */
			while ((input[i] != ' ') && (input[i] != 0))
				i++;

			if (input[i] != 0)
				i++;

			/* Are we writing to EEPROM or MAC */
			switch (input[i]) {
			case ('E'):
				what = EEPROM;
				break;
			case ('M'):
				what = MAC;
				break;
			default:
				what = UNKNOWN;
				break;
			}

			/* skip to the next space or end */
			while ((input[i] != ' ') && (input[i] != 0))
				i++;
			if (input[i] != 0)
				i++;

			/* Find register to write into */
			j = 0;
			while ((input[i] != ' ') && (input[i] != 0)) {
				j = input[i] - 0x30;
				if (j >= 0xA) {
					j -= 0x07;
				}
				reg = (reg * 0x10) + j;
				i++;
			}

			while ((input[i] != ' ') && (input[i] != 0))
				i++;

			if (input[i] != 0)
				i++;
			else
				what = UNKNOWN;

			/* Get the value to write */
			j = 0;
			while ((input[i] != ' ') && (input[i] != 0)) {
				j = input[i] - 0x30;
				if (j >= 0xA) {
					j -= 0x07;
				}
				value = (value * 0x10) + j;
				i++;
			}

			switch (what) {
			case 1:
				printf ("Writing EEPROM register %02x with %04x\n", reg, value);
				write_eeprom_reg (&dev, value, reg);
				break;
			case 2:
				printf ("Writing MAC register bank %i, reg %02x with %04x\n", reg >> 4, reg & 0xE, value);
				SMC_SELECT_BANK (&dev, reg >> 4);
				SMC_outw (&dev, value, reg & 0xE);
				break;
			default:
				printf ("Wrong\n");
				break;
			}
			break;
		case ('D'):
			dump_eeprom (&dev);
			break;
		case ('M'):
			dump_reg (&dev);
			break;
		case ('C'):
			copy_from_eeprom (&dev);
			break;
		case ('P'):
			print_macaddr (&dev);
			break;
		default:
			break;
		}

	}

	return (0);
}

void copy_from_eeprom (struct eth_device *dev)
{
	int i;

	SMC_SELECT_BANK (dev, 1);
	SMC_outw (dev, (SMC_inw (dev, CTL_REG) & !CTL_EEPROM_SELECT) |
		CTL_RELOAD, CTL_REG);
	i = 100;
	while ((SMC_inw (dev, CTL_REG) & CTL_RELOAD) && --i)
		udelay (100);
	if (i == 0) {
		printf ("Timeout Refreshing EEPROM registers\n");
	} else {
		printf ("EEPROM contents copied to MAC\n");
	}

}

void print_macaddr (struct eth_device *dev)
{
	int i, j, k, mac[6];

	printf ("Current MAC Address in SMSC91111 ");
	SMC_SELECT_BANK (dev, 1);
	for (i = 0; i < 5; i++) {
		printf ("%02x:", SMC_inb (dev, ADDR0_REG + i));
	}

	printf ("%02x\n", SMC_inb (dev, ADDR0_REG + 5));

	i = 0;
	for (j = 0x20; j < 0x23; j++) {
		k = read_eeprom_reg (dev, j);
		mac[i] = k & 0xFF;
		i++;
		mac[i] = k >> 8;
		i++;
	}

	printf ("Current MAC Address in EEPROM    ");
	for (i = 0; i < 5; i++)
		printf ("%02x:", mac[i]);
	printf ("%02x\n", mac[5]);

}
void dump_eeprom (struct eth_device *dev)
{
	int j, k;

	printf ("IOS2-0    ");
	for (j = 0; j < 8; j++) {
		printf ("%03x     ", j);
	}
	printf ("\n");

	for (k = 0; k < 4; k++) {
		if (k == 0)
			printf ("CONFIG ");
		if (k == 1)
			printf ("BASE   ");
		if ((k == 2) || (k == 3))
			printf ("       ");
		for (j = 0; j < 0x20; j += 4) {
			printf ("%02x:%04x ", j + k,
				read_eeprom_reg (dev, j + k));
		}
		printf ("\n");
	}

	for (j = 0x20; j < 0x40; j++) {
		if ((j & 0x07) == 0)
			printf ("\n");
		printf ("%02x:%04x ", j, read_eeprom_reg (dev, j));
	}
	printf ("\n");

}

int read_eeprom_reg (struct eth_device *dev, int reg)
{
	int timeout;

	SMC_SELECT_BANK (dev, 2);
	SMC_outw (dev, reg, PTR_REG);

	SMC_SELECT_BANK (dev, 1);
	SMC_outw (dev, SMC_inw (dev, CTL_REG) | CTL_EEPROM_SELECT |
		CTL_RELOAD, CTL_REG);
	timeout = 100;
	while ((SMC_inw (dev, CTL_REG) & CTL_RELOAD) && --timeout)
		udelay (100);
	if (timeout == 0) {
		printf ("Timeout Reading EEPROM register %02x\n", reg);
		return 0;
	}

	return SMC_inw (dev, GP_REG);

}

int write_eeprom_reg (struct eth_device *dev, int value, int reg)
{
	int timeout;

	SMC_SELECT_BANK (dev, 2);
	SMC_outw (dev, reg, PTR_REG);

	SMC_SELECT_BANK (dev, 1);
	SMC_outw (dev, value, GP_REG);
	SMC_outw (dev, SMC_inw (dev, CTL_REG) | CTL_EEPROM_SELECT |
		CTL_STORE, CTL_REG);
	timeout = 100;
	while ((SMC_inw (dev, CTL_REG) & CTL_STORE) && --timeout)
		udelay (100);
	if (timeout == 0) {
		printf ("Timeout Writing EEPROM register %02x\n", reg);
		return 0;
	}

	return 1;

}

void dump_reg (struct eth_device *dev)
{
	int i, j;

	printf ("    ");
	for (j = 0; j < 4; j++) {
		printf ("Bank%i ", j);
	}
	printf ("\n");
	for (i = 0; i < 0xF; i += 2) {
		printf ("%02x  ", i);
		for (j = 0; j < 4; j++) {
			SMC_SELECT_BANK (dev, j);
			printf ("%04x  ", SMC_inw (dev, i));
		}
		printf ("\n");
	}
}
