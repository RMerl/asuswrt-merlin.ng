// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011-2013
 * Texas Instruments, <www.ti.com>
 */

#include <common.h>
#include <i2c.h>
#include <power/tps65217.h>

struct udevice *tps65217_dev __attribute__((section(".data"))) = NULL;

/**
 * tps65217_reg_read() - Generic function that can read a TPS65217 register
 * @src_reg:		 Source register address
 * @src_val:		 Address of destination variable
 * @return:		 0 for success, not 0 on failure.
 */
int tps65217_reg_read(uchar src_reg, uchar *src_val)
{
#ifndef CONFIG_DM_I2C
	return i2c_read(TPS65217_CHIP_PM, src_reg, 1, src_val, 1);
#else
	return dm_i2c_read(tps65217_dev, src_reg,  src_val, 1);
#endif
}

/**
 *  tps65217_reg_write() - Generic function that can write a TPS65217 PMIC
 *			   register or bit field regardless of protection
 *			   level.
 *
 *  @prot_level:	   Register password protection.  Use
 *			   TPS65217_PROT_LEVEL_NONE,
 *			   TPS65217_PROT_LEVEL_1 or TPS65217_PROT_LEVEL_2
 *  @dest_reg:		   Register address to write.
 *  @dest_val:		   Value to write.
 *  @mask:		   Bit mask (8 bits) to be applied.  Function will only
 *			   change bits that are set in the bit mask.
 *
 *  @return:		   0 for success, not 0 on failure, as per the i2c API
 */
int tps65217_reg_write(uchar prot_level, uchar dest_reg, uchar dest_val,
		       uchar mask)
{
	uchar read_val;
	uchar xor_reg;
	int ret;

	/*
	 * If we are affecting only a bit field, read dest_reg and apply the
	 * mask
	 */
	if (mask != TPS65217_MASK_ALL_BITS) {
#ifndef CONFIG_DM_I2C
		ret = i2c_read(TPS65217_CHIP_PM, dest_reg, 1, &read_val, 1);
#else
		ret = dm_i2c_read(tps65217_dev, dest_reg, &read_val, 1);
#endif
		if (ret)
			return ret;

		read_val &= (~mask);
		read_val |= (dest_val & mask);
		dest_val = read_val;
	}

	if (prot_level > 0) {
		xor_reg = dest_reg ^ TPS65217_PASSWORD_UNLOCK;
#ifndef CONFIG_DM_I2C
		ret = i2c_write(TPS65217_CHIP_PM, TPS65217_PASSWORD, 1,
				&xor_reg, 1);
#else
		ret = dm_i2c_write(tps65217_dev, TPS65217_PASSWORD,
				   &xor_reg, 1);
#endif
		if (ret)
			return ret;
	}
#ifndef CONFIG_DM_I2C
	ret = i2c_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1);
#else
	ret = dm_i2c_write(tps65217_dev, dest_reg, &dest_val, 1);
#endif
	if (ret)
		return ret;

	if (prot_level == TPS65217_PROT_LEVEL_2) {
#ifndef CONFIG_DM_I2C
		ret = i2c_write(TPS65217_CHIP_PM, TPS65217_PASSWORD, 1,
				&xor_reg, 1);
#else
		ret = dm_i2c_write(tps65217_dev, TPS65217_PASSWORD,
				   &xor_reg, 1);
#endif
		if (ret)
			return ret;

#ifndef CONFIG_DM_I2C
		ret = i2c_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1);
#else
		ret = dm_i2c_write(tps65217_dev, dest_reg, &dest_val, 1);
#endif
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * tps65217_voltage_update() - Function to change a voltage level, as this
 *			       is a multi-step process.
 * @dc_cntrl_reg:	       DC voltage control register to change.
 * @volt_sel:		       New value for the voltage register
 * @return:		       0 for success, not 0 on failure.
 */
int tps65217_voltage_update(uchar dc_cntrl_reg, uchar volt_sel)
{
	if ((dc_cntrl_reg != TPS65217_DEFDCDC1) &&
	    (dc_cntrl_reg != TPS65217_DEFDCDC2) &&
	    (dc_cntrl_reg != TPS65217_DEFDCDC3))
		return 1;

	/* set voltage level */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, dc_cntrl_reg, volt_sel,
			       TPS65217_MASK_ALL_BITS))
		return 1;

	/* set GO bit to initiate voltage transition */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, TPS65217_DEFSLEW,
			       TPS65217_DCDC_GO, TPS65217_DCDC_GO))
		return 1;

	return 0;
}

int power_tps65217_init(unsigned char bus)
{
#ifdef CONFIG_DM_I2C
	struct udevice *dev = NULL;
	int rc;

	rc = i2c_get_chip_for_busnum(bus, TPS65217_CHIP_PM, 1, &dev);
	if (rc)
		return rc;
	tps65217_dev = dev;
#endif
	return 0;
}
