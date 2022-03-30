/*
 * Voltage Controller implementation for OMAP
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/omap_common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>

/* Register defines and masks for VC IP Block */
/* PRM_VC_CFG_I2C_MODE */
#define PRM_VC_CFG_I2C_MODE_DFILTEREN_BIT	(0x1 << 6)
#define PRM_VC_CFG_I2C_MODE_SRMODEEN_BIT	(0x1 << 4)
#define PRM_VC_CFG_I2C_MODE_HSMODEEN_BIT	(0x1 << 3)
#define PRM_VC_CFG_I2C_MODE_HSMCODE_SHIFT	0x0
#define PRM_VC_CFG_I2C_MODE_HSMCODE_MASK	0x3

/* PRM_VC_CFG_I2C_CLK */
#define PRM_VC_CFG_I2C_CLK_HSCLL_SHIFT		24
#define PRM_VC_CFG_I2C_CLK_HSCLL_MASK		0xFF
#define PRM_VC_CFG_I2C_CLK_HSCLH_SHIFT		16
#define PRM_VC_CFG_I2C_CLK_HSCLH_MASK		0xFF
#define PRM_VC_CFG_I2C_CLK_SCLH_SHIFT		0
#define PRM_VC_CFG_I2C_CLK_SCLH_MASK		0xFF
#define PRM_VC_CFG_I2C_CLK_SCLL_SHIFT		8
#define PRM_VC_CFG_I2C_CLK_SCLL_MASK		(0xFF << 8)

/* PRM_VC_VAL_BYPASS */
#define PRM_VC_VAL_BYPASS_VALID_BIT		(0x1 << 24)
#define PRM_VC_VAL_BYPASS_SLAVEADDR_SHIFT	0
#define PRM_VC_VAL_BYPASS_SLAVEADDR_MASK	0x7F
#define PRM_VC_VAL_BYPASS_REGADDR_SHIFT		8
#define PRM_VC_VAL_BYPASS_REGADDR_MASK		0xFF
#define PRM_VC_VAL_BYPASS_DATA_SHIFT		16
#define PRM_VC_VAL_BYPASS_DATA_MASK		0xFF

/**
 * omap_vc_init() - Initialization for Voltage controller
 * @speed_khz: I2C buspeed in KHz
 */
static void omap_vc_init(u16 speed_khz)
{
	u32 val;
	u32 sys_clk_khz, cycles_hi, cycles_low;

	sys_clk_khz = get_sys_clk_freq() / 1000;

	if (speed_khz > 400) {
		puts("higher speed requested - throttle to 400Khz\n");
		speed_khz = 400;
	}

	/*
	 * Setup the dedicated I2C controller for Voltage Control
	 * I2C clk - high period 40% low period 60%
	 */
	speed_khz /= 10;
	cycles_hi = sys_clk_khz * 4 / speed_khz;
	cycles_low = sys_clk_khz * 6 / speed_khz;
	/* values to be set in register - less by 5 & 7 respectively */
	cycles_hi -= 5;
	cycles_low -= 7;
	val = (cycles_hi << PRM_VC_CFG_I2C_CLK_SCLH_SHIFT) |
	       (cycles_low << PRM_VC_CFG_I2C_CLK_SCLL_SHIFT);
	writel(val, (*prcm)->prm_vc_cfg_i2c_clk);

	/*
	 * Master code if there are multiple masters on the I2C_SR bus.
	 */
	val = 0x0 << PRM_VC_CFG_I2C_MODE_HSMCODE_SHIFT;
	/* No HS mode for now */
	val &= ~PRM_VC_CFG_I2C_MODE_HSMODEEN_BIT;
	writel(val, (*prcm)->prm_vc_cfg_i2c_mode);
}

/**
 * omap_vc_bypass_send_value() - Send a data using VC Bypass command
 * @sa:		7 bit I2C slave address of the PMIC
 * @reg_addr:	I2C register address(8 bit) address in PMIC
 * @reg_data:	what 8 bit data to write
 */
int omap_vc_bypass_send_value(u8 sa, u8 reg_addr, u8 reg_data)
{
	/*
	 * Unfortunately we need to loop here instead of a defined time
	 * use arbitary large value
	 */
	u32 timeout = 0xFFFF;
	u32 reg_val;

	sa &= PRM_VC_VAL_BYPASS_SLAVEADDR_MASK;
	reg_addr &= PRM_VC_VAL_BYPASS_REGADDR_MASK;
	reg_data &= PRM_VC_VAL_BYPASS_DATA_MASK;

	/* program VC to send data */
	reg_val = sa << PRM_VC_VAL_BYPASS_SLAVEADDR_SHIFT |
	    reg_addr << PRM_VC_VAL_BYPASS_REGADDR_SHIFT |
	    reg_data << PRM_VC_VAL_BYPASS_DATA_SHIFT;
	writel(reg_val, (*prcm)->prm_vc_val_bypass);

	/* Signal VC to send data */
	writel(reg_val | PRM_VC_VAL_BYPASS_VALID_BIT,
				(*prcm)->prm_vc_val_bypass);

	/* Wait on VC to complete transmission */
	do {
		reg_val = readl((*prcm)->prm_vc_val_bypass) &
				PRM_VC_VAL_BYPASS_VALID_BIT;
		if (!reg_val)
			break;

		sdelay(100);
	} while (--timeout);

	/* Optional: cleanup PRM_IRQSTATUS_Ax */
	/* In case we can do something about it in future.. */
	if (!timeout)
		return -1;

	/* All good.. */
	return 0;
}

void sri2c_init(void)
{
	static int sri2c = 1;

	if (sri2c) {
		omap_vc_init(PRM_VC_I2C_CHANNEL_FREQ_KHZ);
		sri2c = 0;
	}
	return;
}
