// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Jelle van der Waa <jelle@vdwaa.nl>
 */
#include <common.h>
#include <i2c.h>
#include <sy8106a.h>

#define SY8106A_I2C_ADDR 0x65
#define SY8106A_VOUT1_SEL 1
#define SY8106A_VOUT1_SEL_ENABLE (1 << 7)

#ifdef CONFIG_SPL_BUILD
static u8 sy8106a_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int sy8106a_set_vout1(unsigned int mvolt)
{
	u8 data = sy8106a_mvolt_to_cfg(mvolt, 680, 1950, 10) | SY8106A_VOUT1_SEL_ENABLE;
	return i2c_write(SY8106A_I2C_ADDR, SY8106A_VOUT1_SEL, 1, &data, 1);
}
#endif
