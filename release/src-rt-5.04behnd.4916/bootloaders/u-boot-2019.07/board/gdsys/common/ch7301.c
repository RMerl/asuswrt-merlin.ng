// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

/* Chrontel CH7301C DVI Transmitter */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>

#define CH7301_I2C_ADDR 0x75

enum {
	CH7301_CM = 0x1c,		/* Clock Mode Register */
	CH7301_IC = 0x1d,		/* Input Clock Register */
	CH7301_GPIO = 0x1e,		/* GPIO Control Register */
	CH7301_IDF = 0x1f,		/* Input Data Format Register */
	CH7301_CD = 0x20,		/* Connection Detect Register */
	CH7301_DC = 0x21,		/* DAC Control Register */
	CH7301_HPD = 0x23,		/* Hot Plug Detection Register */
	CH7301_TCTL = 0x31,		/* DVI Control Input Register */
	CH7301_TPCP = 0x33,		/* DVI PLL Charge Pump Ctrl Register */
	CH7301_TPD = 0x34,		/* DVI PLL Divide Register */
	CH7301_TPVT = 0x35,		/* DVI PLL Supply Control Register */
	CH7301_TPF = 0x36,		/* DVI PLL Filter Register */
	CH7301_TCT = 0x37,		/* DVI Clock Test Register */
	CH7301_TSTP = 0x48,		/* Test Pattern Register */
	CH7301_PM = 0x49,		/* Power Management register */
	CH7301_VID = 0x4a,		/* Version ID Register */
	CH7301_DID = 0x4b,		/* Device ID Register */
	CH7301_DSP = 0x56,		/* DVI Sync polarity Register */
};

int ch7301_i2c[] = CONFIG_SYS_CH7301_I2C;

int ch7301_probe(unsigned screen, bool power)
{
	u8 value;

	i2c_set_bus_num(ch7301_i2c[screen]);
	if (i2c_probe(CH7301_I2C_ADDR))
		return -1;

	value = i2c_reg_read(CH7301_I2C_ADDR, CH7301_DID);
	if (value != 0x17)
		return -1;

	if (power) {
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPCP, 0x08);
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPD, 0x16);
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_TPF, 0x60);
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_DC, 0x09);
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_PM, 0xc0);
	} else {
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_DC, 0x00);
		i2c_reg_write(CH7301_I2C_ADDR, CH7301_PM, 0x01);
	}

	return 0;
}

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
