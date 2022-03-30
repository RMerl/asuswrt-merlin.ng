// SPDX-License-Identifier: GPL-2.0+
/*
 * clk-synthesizer.c
 *
 * Clock synthesizer apis
 *
 * Copyright (C) 2016, Texas Instruments, Incorporated - http://www.ti.com/
 */


#include <common.h>
#include <asm/arch/clk_synthesizer.h>
#include <i2c.h>

/**
 * clk_synthesizer_reg_read - Read register from synthesizer.
 * dev:		i2c bus device (not used if CONFIG_DM_I2C is not set)
 * @addr:	addr within the i2c device
 * buf:		Buffer to which value is to be read.
 *
 * For reading the register from this clock synthesizer, a command needs to
 * be send along with enabling byte read more, and then read can happen.
 * Returns 0 on success
 */
static int clk_synthesizer_reg_read(struct udevice *dev, int addr, u8 *buf)
{
	int rc;

	/* Enable Bye read */
	addr = addr | CLK_SYNTHESIZER_BYTE_MODE;

#ifndef CONFIG_DM_I2C
	/* Send the command byte */
	rc = i2c_write(CLK_SYNTHESIZER_I2C_ADDR, addr, 1, buf, 1);
	if (rc)
		printf("Failed to send command to clock synthesizer\n");

	/* Read the Data */
	return i2c_read(CLK_SYNTHESIZER_I2C_ADDR, addr, 1, buf, 1);
#else
	/* Send the command byte */
	rc = dm_i2c_reg_write(dev, addr, *buf);
	if (rc)
		printf("Failed to send command to clock synthesizer\n");

	/* Read the Data */
	rc = dm_i2c_reg_read(dev, addr);
	if (rc < 0)
		return rc;

	*buf = (u8)rc;
	return 0;
#endif

}

/**
 * clk_synthesizer_reg_write - Write a value to register in synthesizer.
 * dev:		i2c bus device (not used if CONFIG_DM_I2C is not set)
 * @addr:	addr within the i2c device
 * val:		Value to be written in the addr.
 *
 * Enable the byte read mode in the address and start the i2c transfer.
 * Returns 0 on success
 */
static int clk_synthesizer_reg_write(struct udevice *dev, int addr, u8 val)
{
	u8 cmd[2];
	int rc = 0;

	/* Enable byte write */
	cmd[0] = addr | CLK_SYNTHESIZER_BYTE_MODE;
	cmd[1] = val;

#ifndef CONFIG_DM_I2C
	rc = i2c_write(CLK_SYNTHESIZER_I2C_ADDR, addr, 1, cmd, 2);
#else
	rc = dm_i2c_write(dev, addr, cmd, 2);
#endif
	if (rc)
		printf("Clock synthesizer reg write failed at addr = 0x%x\n",
		       addr);
	return rc;
}

/**
 * setup_clock_syntherizer - Program the clock synthesizer to get the desired
 *				frequency.
 * @data: Data containing the desired output
 *
 * This is a PLL-based high performance synthesizer which gives 3 outputs
 * as per the PLL_DIV and load capacitor programmed.
 */
int setup_clock_synthesizer(struct clk_synth *data)
{
	int rc;
	u8 val = 0;
	struct udevice *dev = NULL;
#ifndef CONFIG_DM_I2C
	rc =  i2c_probe(CLK_SYNTHESIZER_I2C_ADDR);
	if (rc) {
		printf("i2c probe failed at address 0x%x\n",
		       CLK_SYNTHESIZER_I2C_ADDR);
		return rc;
	}
#else
	rc = i2c_get_chip_for_busnum(0, CLK_SYNTHESIZER_I2C_ADDR, 1, &dev);
	if (rc) {
		printf("failed to get device for synthesizer at address 0x%x\n",
		       CLK_SYNTHESIZER_I2C_ADDR);
		return rc;
	}
#endif
	rc = clk_synthesizer_reg_read(dev, CLK_SYNTHESIZER_ID_REG, &val);
	if (val != data->id)
		return rc;

	/* Crystal Load capacitor selection */
	rc = clk_synthesizer_reg_write(dev, CLK_SYNTHESIZER_XCSEL,
				       data->capacitor);
	if (rc)
		return rc;
	rc = clk_synthesizer_reg_write(dev, CLK_SYNTHESIZER_MUX_REG,
				       data->mux);
	if (rc)
		return rc;
	rc = clk_synthesizer_reg_write(dev, CLK_SYNTHESIZER_PDIV2_REG,
				       data->pdiv2);
	if (rc)
		return rc;
	rc = clk_synthesizer_reg_write(dev, CLK_SYNTHESIZER_PDIV3_REG,
				       data->pdiv3);
	if (rc)
		return rc;

	return 0;
}
