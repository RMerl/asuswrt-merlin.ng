// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 Stefano Babic <sbabic@denx.de>
 */


#include <config.h>
#include <common.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <i2c.h>
#include <mc9sdz60.h>

#ifndef CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR
#error "You have to configure I2C address for MC9SDZ60"
#endif


u8 mc9sdz60_reg_read(enum mc9sdz60_reg reg)
{
	u8 val;

	if (i2c_read(CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR, reg, 1, &val, 1)) {
		puts("Error reading MC9SDZ60 register\n");
		return -1;
	}

	return val;
}

void mc9sdz60_reg_write(enum mc9sdz60_reg reg, u8 val)
{
	i2c_write(CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR, reg, 1, &val, 1);
}
