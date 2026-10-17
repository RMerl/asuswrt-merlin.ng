// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 */

/*
 * Driver for DS4510, a CPU supervisor with integrated EEPROM, SRAM,
 * and 4 programmable non-volatile GPIO pins.
 */

#include <common.h>
#include <i2c.h>
#include <command.h>
#include <linux/delay.h>
#include "ds4510.h"

enum {
	DS4510_CMD_INFO,
	DS4510_CMD_DEVICE,
	DS4510_CMD_NV,
	DS4510_CMD_RSTDELAY,
	DS4510_CMD_OUTPUT,
	DS4510_CMD_INPUT,
	DS4510_CMD_PULLUP,
	DS4510_CMD_EEPROM,
	DS4510_CMD_SEEPROM,
	DS4510_CMD_SRAM,
};

/*
 * Write to DS4510, taking page boundaries into account
 */
static int ds4510_mem_write(uint8_t chip, int offset, uint8_t *buf, int count)
{
	int wrlen;
	int i = 0;

	do {
		wrlen = DS4510_EEPROM_PAGE_SIZE -
			DS4510_EEPROM_PAGE_OFFSET(offset);
		if (count < wrlen)
			wrlen = count;
		if (i2c_write(chip, offset, 1, &buf[i], wrlen))
			return -1;

		/*
		 * This delay isn't needed for SRAM writes but shouldn't delay
		 * things too much, so do it unconditionally for simplicity
		 */
		udelay(DS4510_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
		count -= wrlen;
		offset += wrlen;
		i += wrlen;
	} while (count > 0);

	return 0;
}

/*
 * General read from DS4510
 */
static int ds4510_mem_read(uint8_t chip, int offset, uint8_t *buf, int count)
{
	return i2c_read(chip, offset, 1, buf, count);
}

/*
 * Write SEE bit in config register.
 * nv = 0 - Writes to SEEPROM registers behave like EEPROM
 * nv = 1 - Writes to SEEPROM registers behave like SRAM
 */
static int ds4510_see_write(uint8_t chip, uint8_t nv)
{
	uint8_t data;

	if (i2c_read(chip, DS4510_CFG, 1, &data, 1))
		return -1;

	if (nv)	/* Treat SEEPROM bits as EEPROM */
		data &= ~DS4510_CFG_SEE;
	else	/* Treat SEEPROM bits as SRAM */
		data |= DS4510_CFG_SEE;

	return ds4510_mem_write(chip, DS4510_CFG, &data, 1);
}

/*
 * Write de-assertion of reset signal delay
 */
static int ds4510_rstdelay_write(uint8_t chip, uint8_t delay)
{
	uint8_t data;

	if (i2c_read(chip, DS4510_RSTDELAY, 1, &data, 1))
		return -1;

	data &= ~DS4510_RSTDELAY_MASK;
	data |= delay & DS4510_RSTDELAY_MASK;

	return ds4510_mem_write(chip, DS4510_RSTDELAY, &data, 1);
}

/*
 * Write pullup characteristics of IO pins
 */
static int ds4510_pullup_write(uint8_t chip, uint8_t val)
{
	val &= DS4510_IO_MASK;

	return ds4510_mem_write(chip, DS4510_PULLUP, (uint8_t *)&val, 1);
}

/*
 * Read pullup characteristics of IO pins
 */
static int ds4510_pullup_read(uint8_t chip)
{
	uint8_t val;

	if (i2c_read(chip, DS4510_PULLUP, 1, &val, 1))
		return -1;

	return val & DS4510_IO_MASK;
}

/*
 * Write drive level of IO pins
 */
static int ds4510_gpio_write(uint8_t chip, uint8_t val)
{
	uint8_t data;
	int i;

	for (i = 0; i < DS4510_NUM_IO; i++) {
		if (i2c_read(chip, DS4510_IO0 - i, 1, &data, 1))
			return -1;

		if (val & (0x1 << i))
			data |= 0x1;
		else
			data &= ~0x1;

		if (ds4510_mem_write(chip, DS4510_IO0 - i, &data, 1))
			return -1;
	}

	return 0;
}

/*
 * Read drive level of IO pins
 */
static int ds4510_gpio_read(uint8_t chip)
{
	uint8_t data;
	int val = 0;
	int i;

	for (i = 0; i < DS4510_NUM_IO; i++) {
		if (i2c_read(chip, DS4510_IO0 - i, 1, &data, 1))
			return -1;

		if (data & 1)
			val |= (1 << i);
	}

	return val;
}

/*
 * Read physical level of IO pins
 */
static int ds4510_gpio_read_val(uint8_t chip)
{
	uint8_t val;

	if (i2c_read(chip, DS4510_IO_STATUS, 1, &val, 1))
		return -1;

	return val & DS4510_IO_MASK;
}

/*
 * Display DS4510 information
 */
static int ds4510_info(uint8_t chip)
{
	int i;
	int tmp;
	uint8_t data;

	printf("DS4510 @ 0x%x:\n\n", chip);

	if (i2c_read(chip, DS4510_RSTDELAY, 1, &data, 1))
		return -1;
	printf("rstdelay = 0x%x\n\n", data & DS4510_RSTDELAY_MASK);

	if (i2c_read(chip, DS4510_CFG, 1, &data, 1))
		return -1;
	printf("config   = 0x%x\n", data);
	printf(" /ready  = %d\n", data & DS4510_CFG_READY ? 1 : 0);
	printf(" trip pt = %d\n", data & DS4510_CFG_TRIP_POINT ? 1 : 0);
	printf(" rst sts = %d\n", data & DS4510_CFG_RESET ? 1 : 0);
	printf(" /see    = %d\n", data & DS4510_CFG_SEE ? 1 : 0);
	printf(" swrst   = %d\n\n", data & DS4510_CFG_SWRST ? 1 : 0);

	printf("gpio pins: 3210\n");
	printf("---------------\n");
	printf("pullup     ");

	tmp = ds4510_pullup_read(chip);
	if (tmp == -1)
		return tmp;
	for (i = DS4510_NUM_IO - 1; i >= 0; i--)
		printf("%d", (tmp & (1 << i)) ? 1 : 0);
	printf("\n");

	printf("driven     ");
	tmp = ds4510_gpio_read(chip);
	if (tmp == -1)
		return -1;
	for (i = DS4510_NUM_IO - 1; i >= 0; i--)
		printf("%d", (tmp & (1 << i)) ? 1 : 0);
	printf("\n");

	printf("read       ");
	tmp = ds4510_gpio_read_val(chip);
	if (tmp == -1)
		return -1;
	for (i = DS4510_NUM_IO - 1; i >= 0; i--)
		printf("%d", (tmp & (1 << i)) ? 1 : 0);
	printf("\n");

	return 0;
}

struct cmd_tbl cmd_ds4510[] = {
	U_BOOT_CMD_MKENT(device, 3, 0, (void *)DS4510_CMD_DEVICE, "", ""),
	U_BOOT_CMD_MKENT(nv, 3, 0, (void *)DS4510_CMD_NV, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 0, (void *)DS4510_CMD_OUTPUT, "", ""),
	U_BOOT_CMD_MKENT(input, 3, 0, (void *)DS4510_CMD_INPUT, "", ""),
	U_BOOT_CMD_MKENT(pullup, 4, 0, (void *)DS4510_CMD_PULLUP, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 0, (void *)DS4510_CMD_INFO, "", ""),
	U_BOOT_CMD_MKENT(rstdelay, 3, 0, (void *)DS4510_CMD_RSTDELAY, "", ""),
	U_BOOT_CMD_MKENT(eeprom, 6, 0, (void *)DS4510_CMD_EEPROM, "", ""),
	U_BOOT_CMD_MKENT(seeprom, 6, 0, (void *)DS4510_CMD_SEEPROM, "", ""),
	U_BOOT_CMD_MKENT(sram, 6, 0, (void *)DS4510_CMD_SRAM, "", ""),
};

int do_ds4510(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	static uint8_t chip = 0x51;
	struct cmd_tbl *c;
	ulong ul_arg2 = 0;
	ulong ul_arg3 = 0;
	int tmp;
	ulong addr;
	ulong off;
	ulong cnt;
	int end;
	int (*rw_func)(uint8_t, int, uint8_t *, int);

	c = find_cmd_tbl(argv[1], cmd_ds4510, ARRAY_SIZE(cmd_ds4510));

	/* All commands but "device" require 'maxargs' arguments */
	if (!c || !((argc == (c->maxargs)) ||
		(((int)c->cmd == DS4510_CMD_DEVICE) &&
		 (argc == (c->maxargs - 1))))) {
		return cmd_usage(cmdtp);
	}

	/* arg2 used as chip addr and pin number */
	if (argc > 2)
		ul_arg2 = hextoul(argv[2], NULL);

	/* arg3 used as output/pullup value */
	if (argc > 3)
		ul_arg3 = hextoul(argv[3], NULL);

	switch ((int)c->cmd) {
	case DS4510_CMD_DEVICE:
		if (argc == 3)
			chip = ul_arg2;
		printf("Current device address: 0x%x\n", chip);
		return 0;
	case DS4510_CMD_NV:
		return ds4510_see_write(chip, ul_arg2);
	case DS4510_CMD_OUTPUT:
		tmp = ds4510_gpio_read(chip);
		if (tmp == -1)
			return -1;
		if (ul_arg3)
			tmp |= (1 << ul_arg2);
		else
			tmp &= ~(1 << ul_arg2);
		return ds4510_gpio_write(chip, tmp);
	case DS4510_CMD_INPUT:
		tmp = ds4510_gpio_read_val(chip);
		if (tmp == -1)
			return -1;
		return (tmp & (1 << ul_arg2)) != 0;
	case DS4510_CMD_PULLUP:
		tmp = ds4510_pullup_read(chip);
		if (tmp == -1)
			return -1;
		if (ul_arg3)
			tmp |= (1 << ul_arg2);
		else
			tmp &= ~(1 << ul_arg2);
		return ds4510_pullup_write(chip, tmp);
	case DS4510_CMD_INFO:
		return ds4510_info(chip);
	case DS4510_CMD_RSTDELAY:
		return ds4510_rstdelay_write(chip, ul_arg2);
	case DS4510_CMD_EEPROM:
		end = DS4510_EEPROM + DS4510_EEPROM_SIZE;
		off = DS4510_EEPROM;
		break;
	case DS4510_CMD_SEEPROM:
		end = DS4510_SEEPROM + DS4510_SEEPROM_SIZE;
		off = DS4510_SEEPROM;
		break;
	case DS4510_CMD_SRAM:
		end = DS4510_SRAM + DS4510_SRAM_SIZE;
		off = DS4510_SRAM;
		break;
	default:
		/* We should never get here... */
		return 1;
	}

	/* Only eeprom, seeprom, and sram commands should make it here */
	if (strcmp(argv[2], "read") == 0)
		rw_func = ds4510_mem_read;
	else if (strcmp(argv[2], "write") == 0)
		rw_func = ds4510_mem_write;
	else
		return cmd_usage(cmdtp);

	addr = hextoul(argv[3], NULL);
	off += hextoul(argv[4], NULL);
	cnt = hextoul(argv[5], NULL);

	if ((off + cnt) > end) {
		printf("ERROR: invalid len\n");
		return -1;
	}

	return rw_func(chip, off, (uint8_t *)addr, cnt);
}

U_BOOT_CMD(
	ds4510,	6,	1,	do_ds4510,
	"ds4510 eeprom/seeprom/sram/gpio access",
	"device [dev]\n"
	"	- show or set current device address\n"
	"ds4510 info\n"
	"	- display ds4510 info\n"
	"ds4510 output pin 0|1\n"
	"	- set pin low or high-Z\n"
	"ds4510 input pin\n"
	"	- read value of pin\n"
	"ds4510 pullup pin 0|1\n"
	"	- disable/enable pullup on specified pin\n"
	"ds4510 nv 0|1\n"
	"	- make gpio and seeprom writes volatile/non-volatile"
	"\n"
	"ds4510 rstdelay 0-3\n"
	"	- set reset output delay"
	"\n"
	"ds4510 eeprom read addr off cnt\n"
	"ds4510 eeprom write addr off cnt\n"
	"	- read/write 'cnt' bytes at EEPROM offset 'off'\n"
	"ds4510 seeprom read addr off cnt\n"
	"ds4510 seeprom write addr off cnt\n"
	"	- read/write 'cnt' bytes at SRAM-shadowed EEPROM offset 'off'\n"
	"ds4510 sram read addr off cnt\n"
	"ds4510 sram write addr off cnt\n"
	"	- read/write 'cnt' bytes at SRAM offset 'off'"
);
