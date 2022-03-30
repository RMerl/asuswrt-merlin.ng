// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 */

/*
 * Driver for NXP's 4, 8 and 16 bit I2C gpio expanders (eg pca9537, pca9557,
 * pca9539, etc)
 */

#include <common.h>
#include <i2c.h>
#include <pca953x.h>

/* Default to an address that hopefully won't corrupt other i2c devices */
#ifndef CONFIG_SYS_I2C_PCA953X_ADDR
#define CONFIG_SYS_I2C_PCA953X_ADDR	(~0)
#endif

enum {
	PCA953X_CMD_INFO,
	PCA953X_CMD_DEVICE,
	PCA953X_CMD_OUTPUT,
	PCA953X_CMD_INPUT,
	PCA953X_CMD_INVERT,
};

#ifdef CONFIG_SYS_I2C_PCA953X_WIDTH
struct pca953x_chip_ngpio {
	uint8_t chip;
	uint8_t ngpio;
};

static struct pca953x_chip_ngpio pca953x_chip_ngpios[] =
    CONFIG_SYS_I2C_PCA953X_WIDTH;

/*
 * Determine the number of GPIO pins supported. If we don't know we assume
 * 8 pins.
 */
static int pca953x_ngpio(uint8_t chip)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pca953x_chip_ngpios); i++)
		if (pca953x_chip_ngpios[i].chip == chip)
			return pca953x_chip_ngpios[i].ngpio;

	return 8;
}
#else
static int pca953x_ngpio(uint8_t chip)
{
	return 8;
}
#endif

/*
 * Modify masked bits in register
 */
static int pca953x_reg_write(uint8_t chip, uint addr, uint mask, uint data)
{
	uint8_t valb;
	uint16_t valw;

	if (pca953x_ngpio(chip) <= 8) {
		if (i2c_read(chip, addr, 1, &valb, 1))
			return -1;

		valb &= ~mask;
		valb |= data;

		return i2c_write(chip, addr, 1, &valb, 1);
	} else {
		if (i2c_read(chip, addr << 1, 1, (u8*)&valw, 2))
			return -1;

		valw = le16_to_cpu(valw);
		valw &= ~mask;
		valw |= data;
		valw = cpu_to_le16(valw);

		return i2c_write(chip, addr << 1, 1, (u8*)&valw, 2);
	}
}

static int pca953x_reg_read(uint8_t chip, uint addr, uint *data)
{
	uint8_t valb;
	uint16_t valw;

	if (pca953x_ngpio(chip) <= 8) {
		if (i2c_read(chip, addr, 1, &valb, 1))
			return -1;
		*data = (int)valb;
	} else {
		if (i2c_read(chip, addr << 1, 1, (u8*)&valw, 2))
			return -1;
		*data = (uint)le16_to_cpu(valw);
	}
	return 0;
}

/*
 * Set output value of IO pins in 'mask' to corresponding value in 'data'
 * 0 = low, 1 = high
 */
int pca953x_set_val(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_OUT, mask, data);
}

/*
 * Set read polarity of IO pins in 'mask' to corresponding value in 'data'
 * 0 = read pin value, 1 = read inverted pin value
 */
int pca953x_set_pol(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_POL, mask, data);
}

/*
 * Set direction of IO pins in 'mask' to corresponding value in 'data'
 * 0 = output, 1 = input
 */
int pca953x_set_dir(uint8_t chip, uint mask, uint data)
{
	return pca953x_reg_write(chip, PCA953X_CONF, mask, data);
}

/*
 * Read current logic level of all IO pins
 */
int pca953x_get_val(uint8_t chip)
{
	uint val;

	if (pca953x_reg_read(chip, PCA953X_IN, &val) < 0)
		return -1;

	return (int)val;
}

#if defined(CONFIG_CMD_PCA953X) && !defined(CONFIG_SPL_BUILD)
/*
 * Display pca953x information
 */
static int pca953x_info(uint8_t chip)
{
	int i;
	uint data;
	int nr_gpio = pca953x_ngpio(chip);
	int msb = nr_gpio - 1;

	printf("pca953x@ 0x%x (%d pins):\n\n", chip, nr_gpio);
	printf("gpio pins: ");
	for (i = msb; i >= 0; i--)
		printf("%x", i);
	printf("\n");
	for (i = 11 + nr_gpio; i > 0; i--)
		printf("-");
	printf("\n");

	if (pca953x_reg_read(chip, PCA953X_CONF, &data) < 0)
		return -1;
	printf("conf:      ");
	for (i = msb; i >= 0; i--)
		printf("%c", data & (1 << i) ? 'i' : 'o');
	printf("\n");

	if (pca953x_reg_read(chip, PCA953X_POL, &data) < 0)
		return -1;
	printf("invert:    ");
	for (i = msb; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	if (pca953x_reg_read(chip, PCA953X_IN, &data) < 0)
		return -1;
	printf("input:     ");
	for (i = msb; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	if (pca953x_reg_read(chip, PCA953X_OUT, &data) < 0)
		return -1;
	printf("output:    ");
	for (i = msb; i >= 0; i--)
		printf("%c", data & (1 << i) ? '1' : '0');
	printf("\n");

	return 0;
}

static cmd_tbl_t cmd_pca953x[] = {
	U_BOOT_CMD_MKENT(device, 3, 0, (void *)PCA953X_CMD_DEVICE, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 0, (void *)PCA953X_CMD_OUTPUT, "", ""),
	U_BOOT_CMD_MKENT(input, 3, 0, (void *)PCA953X_CMD_INPUT, "", ""),
	U_BOOT_CMD_MKENT(invert, 4, 0, (void *)PCA953X_CMD_INVERT, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 0, (void *)PCA953X_CMD_INFO, "", ""),
};

static int do_pca953x(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static uint8_t chip = CONFIG_SYS_I2C_PCA953X_ADDR;
	int ret = CMD_RET_USAGE, val;
	ulong ul_arg2 = 0;
	ulong ul_arg3 = 0;
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[1], cmd_pca953x, ARRAY_SIZE(cmd_pca953x));

	/* All commands but "device" require 'maxargs' arguments */
	if (!c || !((argc == (c->maxargs)) ||
		(((long)c->cmd == PCA953X_CMD_DEVICE) &&
		 (argc == (c->maxargs - 1))))) {
		return CMD_RET_USAGE;
	}

	/* arg2 used as chip number or pin number */
	if (argc > 2)
		ul_arg2 = simple_strtoul(argv[2], NULL, 16);

	/* arg3 used as pin or invert value */
	if (argc > 3)
		ul_arg3 = simple_strtoul(argv[3], NULL, 16) & 0x1;

	switch ((long)c->cmd) {
	case PCA953X_CMD_INFO:
		ret = pca953x_info(chip);
		if (ret)
			ret = CMD_RET_FAILURE;
		break;

	case PCA953X_CMD_DEVICE:
		if (argc == 3)
			chip = (uint8_t)ul_arg2;
		printf("Current device address: 0x%x\n", chip);
		ret = CMD_RET_SUCCESS;
		break;

	case PCA953X_CMD_INPUT:
		ret = pca953x_set_dir(chip, (1 << ul_arg2),
				PCA953X_DIR_IN << ul_arg2);
		val = (pca953x_get_val(chip) & (1 << ul_arg2)) != 0;

		if (ret)
			ret = CMD_RET_FAILURE;
		else
			printf("chip 0x%02x, pin 0x%lx = %d\n", chip, ul_arg2,
									val);
		break;

	case PCA953X_CMD_OUTPUT:
		ret = pca953x_set_dir(chip, (1 << ul_arg2),
				(PCA953X_DIR_OUT << ul_arg2));
		if (!ret)
			ret = pca953x_set_val(chip, (1 << ul_arg2),
						(ul_arg3 << ul_arg2));
		if (ret)
			ret = CMD_RET_FAILURE;
		break;

	case PCA953X_CMD_INVERT:
		ret = pca953x_set_pol(chip, (1 << ul_arg2),
					(ul_arg3 << ul_arg2));
		if (ret)
			ret = CMD_RET_FAILURE;
		break;
	}

	if (ret == CMD_RET_FAILURE)
		eprintf("Error talking to chip at 0x%x\n", chip);

	return ret;
}

U_BOOT_CMD(
	pca953x,	5,	1,	do_pca953x,
	"pca953x gpio access",
	"device [dev]\n"
	"	- show or set current device address\n"
	"pca953x info\n"
	"	- display info for current chip\n"
	"pca953x output pin 0|1\n"
	"	- set pin as output and drive low or high\n"
	"pca953x invert pin 0|1\n"
	"	- disable/enable polarity inversion for reads\n"
	"pca953x input pin\n"
	"	- set pin as input and read value"
);

#endif /* CONFIG_CMD_PCA953X */
