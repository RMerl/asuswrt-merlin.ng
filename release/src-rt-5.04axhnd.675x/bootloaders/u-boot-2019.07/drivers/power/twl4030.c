// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix at windriver.com>
 *
 * twl4030_power_reset_init is derived from code on omapzoom,
 * git://git.omapzoom.com/repo/u-boot.git
 *
 * Copyright (C) 2007-2009 Texas Instruments, Inc.
 *
 * twl4030_power_init is from cpu/omap3/common.c, power_init_r
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05 at gmail.com>
 *	Shashi Ranjan <shashiranjanmca05 at gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2 at ti.com>
 *	Syed Mohammed Khasim <khasim at ti.com>
 */

#include <twl4030.h>

/*
 * Power Reset
 */
void twl4030_power_reset_init(void)
{
	u8 val = 0;
	if (twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
				TWL4030_PM_MASTER_P1_SW_EVENTS, &val)) {
		printf("Error:TWL4030: failed to read the power register\n");
		printf("Could not initialize hardware reset\n");
	} else {
		val |= TWL4030_PM_MASTER_SW_EVENTS_STOPON_PWRON;
		if (twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
					 TWL4030_PM_MASTER_P1_SW_EVENTS, val)) {
			printf("Error:TWL4030: failed to write the power register\n");
			printf("Could not initialize hardware reset\n");
		}
	}
}

/*
 * Power off
 */
void twl4030_power_off(void)
{
	u8 data;

	/* PM master unlock (CFG and TST keys) */

	data = 0xCE;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, data);
	data = 0xEC;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, data);

	/* VBAT start disable */

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_CFG_P1_TRANSITION, &data);
	data &= ~TWL4030_PM_MASTER_CFG_TRANSITION_STARTON_VBAT;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_CFG_P1_TRANSITION, data);

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_CFG_P2_TRANSITION, &data);
	data &= ~TWL4030_PM_MASTER_CFG_TRANSITION_STARTON_VBAT;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_CFG_P2_TRANSITION, data);

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_CFG_P3_TRANSITION, &data);
	data &= ~TWL4030_PM_MASTER_CFG_TRANSITION_STARTON_VBAT;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_CFG_P3_TRANSITION, data);

	/* High jitter for PWRANA2 */

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_CFG_PWRANA2, &data);
	data &= ~(TWL4030_PM_MASTER_CFG_PWRANA2_LOJIT0_LOWV |
		  TWL4030_PM_MASTER_CFG_PWRANA2_LOJIT1_LOWV);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_CFG_PWRANA2, data);

	/* PM master lock */

	data = 0xFF;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PROTECT_KEY, data);

	/* Power off */

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_P1_SW_EVENTS, &data);
	data |= TWL4030_PM_MASTER_SW_EVENTS_DEVOFF;
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_P1_SW_EVENTS, data);
}

/*
 * Set Device Group and Voltage
 */
void twl4030_pmrecv_vsel_cfg(u8 vsel_reg, u8 vsel_val,
				u8 dev_grp, u8 dev_grp_sel)
{
	int ret;

	/* Select the Voltage */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, vsel_reg,
				   vsel_val);
	if (ret != 0) {
		printf("Could not write vsel to reg %02x (%d)\n",
			vsel_reg, ret);
		return;
	}

	/* Select the Device Group (enable the supply if dev_grp_sel != 0) */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, dev_grp,
				   dev_grp_sel);
	if (ret != 0)
		printf("Could not write grp_sel to reg %02x (%d)\n",
			dev_grp, ret);
}

void twl4030_power_init(void)
{
	/* set VAUX3 to 2.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,
				TWL4030_PM_RECEIVER_VAUX3_VSEL_28,
				TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);

	/* set VPLL2 to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VPLL2_DEDICATED,
				TWL4030_PM_RECEIVER_VPLL2_VSEL_18,
				TWL4030_PM_RECEIVER_VPLL2_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_ALL);

	/* set VDAC to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VDAC_DEDICATED,
				TWL4030_PM_RECEIVER_VDAC_VSEL_18,
				TWL4030_PM_RECEIVER_VDAC_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);
}

void twl4030_power_mmc_init(int dev_index)
{
	if (dev_index == 0) {
		/* Set VMMC1 to 3.15 Volts */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VMMC1_DEDICATED,
					TWL4030_PM_RECEIVER_VMMC1_VSEL_32,
					TWL4030_PM_RECEIVER_VMMC1_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);

		mdelay(100);	/* ramp-up delay from Linux code */
	} else if (dev_index == 1) {
		/* Set VMMC2 to 3.15 Volts */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VMMC2_DEDICATED,
					TWL4030_PM_RECEIVER_VMMC2_VSEL_32,
					TWL4030_PM_RECEIVER_VMMC2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);

		mdelay(100);	/* ramp-up delay from Linux code */
	}
}

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	twl4030_power_off();

	return 0;
}
#endif

#ifdef CONFIG_DM_I2C
int twl4030_i2c_write_u8(u8 chip_no, u8 reg, u8 val)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, chip_no, 1, &dev);
	if (ret) {
		pr_err("unable to get I2C bus. ret %d\n", ret);
		return ret;
	}
	ret = dm_i2c_reg_write(dev, reg, val);
	if (ret) {
		pr_err("writing to twl4030 failed. ret %d\n", ret);
		return ret;
	}
	return 0;
}

int twl4030_i2c_read_u8(u8 chip_no, u8 reg, u8 *valp)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, chip_no, 1, &dev);
	if (ret) {
		pr_err("unable to get I2C bus. ret %d\n", ret);
		return ret;
	}
	ret = dm_i2c_reg_read(dev, reg);
	if (ret < 0) {
		pr_err("reading from twl4030 failed. ret %d\n", ret);
		return ret;
	}
	*valp = (u8)ret;
	return 0;
}
#endif
