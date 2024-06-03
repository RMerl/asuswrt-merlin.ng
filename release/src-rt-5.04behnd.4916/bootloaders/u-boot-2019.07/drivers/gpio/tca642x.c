/*
 * Copyright 2013 Texas Instruments, Inc.
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from the pca953x.c driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <tca642x.h>

/* tca642x register address definitions */
struct tca642x_bank_info tca642x_regs[] = {
	{ .input_reg = 0x00,
	  .output_reg = 0x04,
	  .polarity_reg = 0x08,
	  .configuration_reg = 0x0c },
	{ .input_reg = 0x01,
	  .output_reg = 0x05,
	  .polarity_reg = 0x09,
	  .configuration_reg = 0x0d },
	{ .input_reg = 0x02,
	  .output_reg = 0x06,
	  .polarity_reg = 0x0a,
	  .configuration_reg = 0x0e },
};

/*
 * Modify masked bits in register
 */
static int tca642x_reg_write(uchar chip, uint8_t addr,
		uint8_t reg_bit, uint8_t data)
{
	uint8_t valw;
	int org_bus_num;
	int ret;

	org_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_I2C_TCA642X_BUS_NUM);

	if (i2c_read(chip, addr, 1, (uint8_t *)&valw, 1)) {
		printf("Could not read before writing\n");
		ret = -1;
		goto error;
	}
	valw &= ~reg_bit;
	valw |= data;

	ret = i2c_write(chip, addr, 1, (u8 *)&valw, 1);

error:
	i2c_set_bus_num(org_bus_num);
	return ret;
}

static int tca642x_reg_read(uchar chip, uint8_t addr, uint8_t *data)
{
	uint8_t valw;
	int org_bus_num;
	int ret = 0;

	org_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_I2C_TCA642X_BUS_NUM);
	if (i2c_read(chip, addr, 1, (u8 *)&valw, 1)) {
		ret = -1;
		goto error;
	}

	*data = valw;

error:
	i2c_set_bus_num(org_bus_num);
	return ret;
}

/*
 * Set output value of IO pins in 'reg_bit' to corresponding value in 'data'
 * 0 = low, 1 = high
 */
int tca642x_set_val(uchar chip, uint8_t gpio_bank,
					uint8_t reg_bit, uint8_t data)
{
	uint8_t out_reg = tca642x_regs[gpio_bank].output_reg;

	return tca642x_reg_write(chip, out_reg, reg_bit, data);
}

/*
 * Set read polarity of IO pins in 'reg_bit' to corresponding value in 'data'
 * 0 = read pin value, 1 = read inverted pin value
 */
int tca642x_set_pol(uchar chip, uint8_t gpio_bank,
					uint8_t reg_bit, uint8_t data)
{
	uint8_t pol_reg = tca642x_regs[gpio_bank].polarity_reg;

	return tca642x_reg_write(chip, pol_reg, reg_bit, data);
}

/*
 * Set direction of IO pins in 'reg_bit' to corresponding value in 'data'
 * 0 = output, 1 = input
 */
int tca642x_set_dir(uchar chip, uint8_t gpio_bank,
					uint8_t reg_bit, uint8_t data)
{
	uint8_t config_reg = tca642x_regs[gpio_bank].configuration_reg;

	return tca642x_reg_write(chip, config_reg, reg_bit, data);
}

/*
 * Read current logic level of all IO pins
 */
int tca642x_get_val(uchar chip, uint8_t gpio_bank)
{
	uint8_t val;
	uint8_t in_reg = tca642x_regs[gpio_bank].input_reg;

	if (tca642x_reg_read(chip, in_reg, &val) < 0)
		return -1;

	return (int)val;
}

/*
 * Set the inital register states for the tca642x gpio expander
 */
int tca642x_set_inital_state(uchar chip, struct tca642x_bank_info init_data[])
{
	int i, ret;
	uint8_t config_reg;
	uint8_t polarity_reg;
	uint8_t output_reg;

	for (i = 0; i < 3; i++) {
		config_reg = tca642x_regs[i].configuration_reg;
		ret = tca642x_reg_write(chip, config_reg, 0xff,
				init_data[i].configuration_reg);
		polarity_reg = tca642x_regs[i].polarity_reg;
		ret = tca642x_reg_write(chip, polarity_reg, 0xff,
				init_data[i].polarity_reg);
		output_reg = tca642x_regs[i].output_reg;
		ret = tca642x_reg_write(chip, output_reg, 0xff,
				init_data[i].output_reg);
	}

	return ret;
}

#if defined(CONFIG_CMD_TCA642X) && !defined(CONFIG_SPL_BUILD)
/*
 * Display tca642x information
 */
static int tca642x_info(uchar chip)
{
	int i, j;
	uint8_t data;

	printf("tca642x@ 0x%x (%d pins):\n", chip, 24);
	for (i = 0; i < 3; i++) {
		printf("Bank %i\n", i);
		if (tca642x_reg_read(chip,
				     tca642x_regs[i].configuration_reg,
				     &data) < 0)
			return -1;
		printf("\tConfiguration: ");
		for (j = 7; j >= 0; j--)
			printf("%c", data & (1 << j) ? 'i' : 'o');
		printf("\n");

		if (tca642x_reg_read(chip,
				     tca642x_regs[i].polarity_reg, &data) < 0)
			return -1;
		printf("\tPolarity: ");
		for (j = 7; j >= 0; j--)
			printf("%c", data & (1 << j) ? '1' : '0');
		printf("\n");

		if (tca642x_reg_read(chip,
				     tca642x_regs[i].input_reg, &data) < 0)
			return -1;
		printf("\tInput value: ");
		for (j = 7; j >= 0; j--)
			printf("%c", data & (1 << j) ? '1' : '0');
		printf("\n");

		if (tca642x_reg_read(chip,
				     tca642x_regs[i].output_reg, &data) < 0)
			return -1;
		printf("\tOutput value: ");
		for (j = 7; j >= 0; j--)
			printf("%c", data & (1 << j) ? '1' : '0');
		printf("\n");
	}

	return 0;
}

static cmd_tbl_t cmd_tca642x[] = {
	U_BOOT_CMD_MKENT(device, 3, 0, (void *)TCA642X_CMD_DEVICE, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 0, (void *)TCA642X_CMD_OUTPUT, "", ""),
	U_BOOT_CMD_MKENT(input, 3, 0, (void *)TCA642X_CMD_INPUT, "", ""),
	U_BOOT_CMD_MKENT(invert, 4, 0, (void *)TCA642X_CMD_INVERT, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 0, (void *)TCA642X_CMD_INFO, "", ""),
};

static int do_tca642x(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static uchar chip = CONFIG_SYS_I2C_TCA642X_ADDR;
	int ret = CMD_RET_USAGE, val;
	uint8_t gpio_bank = 0;
	uint8_t bank_shift;
	ulong ul_arg2 = 0;
	ulong ul_arg3 = 0;
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[1], cmd_tca642x, ARRAY_SIZE(cmd_tca642x));

	/* All commands but "device" require 'maxargs' arguments */
	if (!c ||
	    !((argc == (c->maxargs)) ||
	    (((int)c->cmd == TCA642X_CMD_DEVICE) &&
	    (argc == (c->maxargs - 1))))) {
		return CMD_RET_USAGE;
	}

	/* arg2 used as chip number or pin number */
	if (argc > 2)
		ul_arg2 = simple_strtoul(argv[2], NULL, 10);

	/* arg3 used as pin or invert value */
	if (argc > 3) {
		ul_arg3 = simple_strtoul(argv[3], NULL, 10) & 0x1;
		if (ul_arg2 <= 7) {
			gpio_bank = 0;
		} else if ((ul_arg2 >= 10) && (ul_arg2 <= 17)) {
			gpio_bank = 1;
		} else if ((ul_arg2 >= 20) && (ul_arg2 <= 27)) {
			gpio_bank = 2;
		} else {
			printf("Requested pin is not available\n");
			ret = CMD_RET_FAILURE;
			goto error;
		}
	}

	switch ((int)c->cmd) {
	case TCA642X_CMD_INFO:
		ret = tca642x_info(chip);
		if (ret)
			ret = CMD_RET_FAILURE;
		break;

	case TCA642X_CMD_DEVICE:
		if (argc == 3)
			chip = (uint8_t)ul_arg2;
		printf("Current device address: 0x%x\n", chip);
		ret = CMD_RET_SUCCESS;
		break;

	case TCA642X_CMD_INPUT:
		bank_shift = ul_arg2 - (gpio_bank * 10);
		ret = tca642x_set_dir(chip, gpio_bank, (1 << bank_shift),
				TCA642X_DIR_IN << bank_shift);
		val = (tca642x_get_val(chip, gpio_bank) &
				(1 << bank_shift)) != 0;

		if (ret)
			ret = CMD_RET_FAILURE;
		else
			printf("chip 0x%02x, pin 0x%lx = %d\n", chip,
			       ul_arg2, val);
		break;

	case TCA642X_CMD_OUTPUT:
		bank_shift = ul_arg2 - (gpio_bank * 10);
		ret = tca642x_set_dir(chip, gpio_bank, (1 << bank_shift),
				(TCA642X_DIR_OUT << bank_shift));
		if (!ret)
			ret = tca642x_set_val(chip,
					      gpio_bank, (1 << bank_shift),
					      (ul_arg3 << bank_shift));
		if (ret)
			ret = CMD_RET_FAILURE;
		break;

	case TCA642X_CMD_INVERT:
		bank_shift = ul_arg2 - (gpio_bank * 10);
		ret = tca642x_set_pol(chip, gpio_bank, (1 << bank_shift),
					(ul_arg3 << bank_shift));
		if (ret)
			ret = CMD_RET_FAILURE;
		break;
	}
error:
	if (ret == CMD_RET_FAILURE)
		eprintf("Error talking to chip at 0x%x\n", chip);

	return ret;
}

U_BOOT_CMD(
	tca642x,	5,	1,	do_tca642x,
	"tca642x gpio access",
	"device [dev]\n"
	"	- show or set current device address\n"
	"tca642x info\n"
	"	- display info for current chip\n"
	"tca642x output pin 0|1\n"
	"	- set pin as output and drive low or high\n"
	"tca642x invert pin 0|1\n"
	"	- disable/enable polarity inversion for reads\n"
	"tca642x input pin\n"
	"	- set pin as input and read value"
);

#endif /* CONFIG_CMD_TCA642X */
