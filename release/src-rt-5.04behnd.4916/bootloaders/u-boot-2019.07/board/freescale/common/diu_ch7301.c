// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Authors: Priyanka Jain <Priyanka.Jain@freescale.com>
 *	    Wang Dongsheng <dongsheng.wang@freescale.com>
 *
 * This file is copied and modified from the original t1040qds/diu.c.
 * Encoder can be used in T104x and LSx Platform.
 */

#include <common.h>
#include <stdio_dev.h>
#include <i2c.h>

#define I2C_DVI_INPUT_DATA_FORMAT_REG		0x1F
#define I2C_DVI_PLL_CHARGE_CNTL_REG		0x33
#define I2C_DVI_PLL_DIVIDER_REG			0x34
#define I2C_DVI_PLL_SUPPLY_CNTL_REG		0x35
#define I2C_DVI_PLL_FILTER_REG			0x36
#define I2C_DVI_TEST_PATTERN_REG		0x48
#define I2C_DVI_POWER_MGMT_REG			0x49
#define I2C_DVI_LOCK_STATE_REG			0x4D
#define I2C_DVI_SYNC_POLARITY_REG		0x56

/*
 * Set VSYNC/HSYNC to active high. This is polarity of sync signals
 * from DIU->DVI. The DIU default is active igh, so DVI is set to
 * active high.
 */
#define I2C_DVI_INPUT_DATA_FORMAT_VAL		0x98

#define I2C_DVI_PLL_CHARGE_CNTL_HIGH_SPEED_VAL	0x06
#define I2C_DVI_PLL_DIVIDER_HIGH_SPEED_VAL	0x26
#define I2C_DVI_PLL_FILTER_HIGH_SPEED_VAL	0xA0
#define I2C_DVI_PLL_CHARGE_CNTL_LOW_SPEED_VAL	0x08
#define I2C_DVI_PLL_DIVIDER_LOW_SPEED_VAL	0x16
#define I2C_DVI_PLL_FILTER_LOW_SPEED_VAL	0x60

/* Clear test pattern */
#define I2C_DVI_TEST_PATTERN_VAL		0x18
/* Exit Power-down mode */
#define I2C_DVI_POWER_MGMT_VAL			0xC0

/* Monitor polarity is handled via DVI Sync Polarity Register */
#define I2C_DVI_SYNC_POLARITY_VAL		0x00

/* Programming of HDMI Chrontel CH7301 connector */
int diu_set_dvi_encoder(unsigned int pixclock)
{
	int ret;
	u8 temp;

	temp = I2C_DVI_TEST_PATTERN_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_TEST_PATTERN_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select proper dvi test pattern\n");
		return ret;
	}
	temp = I2C_DVI_INPUT_DATA_FORMAT_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_INPUT_DATA_FORMAT_REG,
			1, &temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi input data format\n");
		return ret;
	}

	/* Set Sync polarity register */
	temp = I2C_DVI_SYNC_POLARITY_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_SYNC_POLARITY_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi syc polarity\n");
		return ret;
	}

	/* Set PLL registers based on pixel clock rate*/
	if (pixclock > 65000000) {
		temp = I2C_DVI_PLL_CHARGE_CNTL_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_CHARGE_CNTL_REG, 1,	&temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll charge_cntl\n");
			return ret;
		}
		temp = I2C_DVI_PLL_DIVIDER_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_DIVIDER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll divider\n");
			return ret;
		}
		temp = I2C_DVI_PLL_FILTER_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_FILTER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll filter\n");
			return ret;
		}
	} else {
		temp = I2C_DVI_PLL_CHARGE_CNTL_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_CHARGE_CNTL_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll charge_cntl\n");
			return ret;
		}
		temp = I2C_DVI_PLL_DIVIDER_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_DIVIDER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll divider\n");
			return ret;
		}
		temp = I2C_DVI_PLL_FILTER_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_FILTER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll filter\n");
			return ret;
		}
	}

	temp = I2C_DVI_POWER_MGMT_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_POWER_MGMT_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi power mgmt\n");
		return ret;
	}

	udelay(500);

	return 0;
}
